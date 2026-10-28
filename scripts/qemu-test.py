#!/usr/bin/env python3
"""ankaros imajını QEMU'da ekransız açar, seri konsoldan komut çalıştırır ve ekran görüntüsü alır.

Kullanım:
    ./scripts/qemu-test.py --expect-desktop --wait 120 --screenshot masaustu.png
    ./scripts/qemu-test.py --command "ankaros-shell-preview" --wait 20 --screenshot ekran.ppm
    ./scripts/qemu-test.py --wait 15 --actions "click 326 860; sleep 10" --screenshot ekran.png

Adımlar: imaj açılır (değişiklikler kaydedilmez, -snapshot), giriş isteminde root olarak girilir,
--command arka planda çalıştırılır, --wait saniye beklenir, --actions sırayla uygulanır (tıklama, sürükleme,
tuş, yazı, seri konsolda komut, ara ekran görüntüsü, bkz. run_actions), QEMU'nun screendump komutuyla ekran
görüntüsü alınır ve sanal makine kapatılır. Seri konsol çıktısı --log dosyasına yazılır.

--expect-desktop ile açılış testi yapılır (CI'da bu kullanılır): --wait süresi boyunca birkaç saniyede bir
ekran görüntüsü alınır, ekran masaüstüne benzediğinde test geçer ve açılış süresi raporu (ankarosctl
boot) yazdırılır. Masaüstü hiç gelmezse betik 1 ile çıkar. Kapanmadan önce compositor'ın günlüğü de seri konsol günlüğüne eklenir.

Ekran görüntüsü .ppm ya da .png olabilir (PNG'ye dönüştürme ek paket istemez).

GPU 2D sanal karttır (virtio-gpu-pci), Mesa yazılımsal çizer (softpipe). Hız ölçmek için değil,
"ekranda doğru şey görünüyor mu" sorusunu cevaplamak için.
"""

import argparse
import json
import os
import platform
import socket
import struct
import subprocess
import sys
import tempfile
import time
import zlib


def default_images():
    """Mac'te OrbStack'teki derleme makinesinin çıktısı, Linux'ta (derleme makinesi ve CI) doğrudan."""
    if platform.system() == "Darwin":
        user = os.environ.get("USER", "")
        return os.path.expanduser(f"~/OrbStack/ankaros-build/home/{user}/ankaros-output/images")
    return os.path.expanduser("~/ankaros-output/images")


def wait_for(sock, needle, timeout, log):
    """Seri konsoldan needle görünene kadar oku. Okunan her şeyi log'a yazar."""
    deadline = time.time() + timeout
    buffer = b""
    sock.settimeout(1)
    while time.time() < deadline:
        try:
            data = sock.recv(4096)
        except socket.timeout:
            continue
        if not data:
            break
        log.write(data)
        log.flush()
        buffer += data
        if needle in buffer:
            return True
    return False


def capture(sock, needle, timeout, log):
    """wait_for gibi, ama okunanları da döner (komut çıktısını almak için)."""
    deadline = time.time() + timeout
    buffer = b""
    sock.settimeout(1)
    while time.time() < deadline and needle not in buffer:
        try:
            data = sock.recv(4096)
        except socket.timeout:
            continue
        if not data:
            break
        log.write(data)
        log.flush()
        buffer += data
    # needle'dan sonra satırın geri kalanını da al
    if needle in buffer and not buffer.endswith(b"\n"):
        buffer += capture(sock, b"\n", 2, log)
    return buffer


def print_boot_report(console, log):
    """ankarosctl boot'un açılış raporunu yazdırır (docs/acilis-suresi.md)."""
    console.sendall(b"ankarosctl boot\n")
    output = capture(console, b"Toplam:", 15, log).decode(errors="replace").replace("\r", "")
    start = output.find("ADIM")
    if start < 0 or "Toplam:" not in output:
        print("Açılış raporu alınamadı", file=sys.stderr)
        return
    print(output[start:].strip(), flush=True)


def qmp(path, command, arguments=None):
    """QEMU Machine Protocol üzerinden tek bir komut gönderir."""
    with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
        s.connect(path)
        f = s.makefile("rw")
        f.readline()  # karşılama
        f.write(json.dumps({"execute": "qmp_capabilities"}) + "\n")
        f.flush()
        f.readline()
        message = {"execute": command}
        if arguments:
            message["arguments"] = arguments
        f.write(json.dumps(message) + "\n")
        f.flush()
        return json.loads(f.readline())


SCREEN_WIDTH, SCREEN_HEIGHT = 1440, 900


def move(path, x, y):
    """Sanal tablette (mutlak koordinatlı fare) imleci 1440x900 koordinatlarına taşır."""
    def axis(name, value, size):
        return {"type": "abs", "data": {"axis": name, "value": int(value * 32767 / (size - 1))}}

    qmp(path, "input-send-event", {"events": [axis("x", x, SCREEN_WIDTH), axis("y", y, SCREEN_HEIGHT)]})


def button(path, down):
    qmp(path, "input-send-event", {"events": [{"type": "btn", "data": {"down": down, "button": "left"}}]})


def click(path, x, y):
    """Sol tıklama. İmleç taşındıktan sonra beklenir: softpipe ile bir kare yüzlerce milisaniye
    sürebiliyor ve hareket işlenmeden basılırsa tıklama uygulamaya imlecin eski konumuyla ulaşıyor."""
    move(path, x, y)
    time.sleep(0.6)
    for down in (True, False):
        button(path, down)
        time.sleep(0.08)


def drag(path, x1, y1, x2, y2, steps=12, release=True):
    """Sol düğme basılıyken (x1, y1)'den (x2, y2)'ye adım adım sürükler. release=False ise düğme
    basılı kalır (ör. bırakmadan önce ekran görüntüsü almak için), sonra "up" ile bırakılır."""
    move(path, x1, y1)
    time.sleep(0.6)
    button(path, True)
    for step in range(1, steps + 1):
        time.sleep(0.05)
        move(path, x1 + (x2 - x1) * step / steps, y1 + (y2 - y1) * step / steps)
    time.sleep(0.2)
    if release:
        button(path, False)


def hold_key(path, key, down):
    """Tek bir tuşu basılı tutar ya da bırakır (ör. Alt+Tab için Alt)."""
    event = {"type": "key", "data": {"down": down, "key": {"type": "qcode", "data": key}}}
    qmp(path, "input-send-event", {"events": [event]})
    time.sleep(0.08)


def read_ppm(path):
    """QEMU'nun screendump çıktısını (P6 PPM) okur: (genişlik, yükseklik, RGB baytları)."""
    with open(path, "rb") as f:
        data = f.read()
    # Başlık: "P6", genişlik, yükseklik, en büyük değer, ardından tek bir boşluk ve pikseller
    fields = data.split(maxsplit=4)
    if fields[0] != b"P6":
        raise ValueError("PPM (P6) değil")
    return int(fields[1]), int(fields[2]), fields[4]


def write_png(path, width, height, rgb):
    def chunk(kind, body):
        return (struct.pack(">I", len(body)) + kind + body
                + struct.pack(">I", zlib.crc32(kind + body) & 0xFFFFFFFF))

    stride = width * 3
    rows = b"".join(b"\x00" + rgb[y * stride:(y + 1) * stride] for y in range(height))
    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)))
        f.write(chunk(b"IDAT", zlib.compress(rows, 6)))
        f.write(chunk(b"IEND", b""))


def looks_like_desktop(width, height, rgb):
    """Ekran masaüstüne benziyor mu? Açılışta ekran siyah zemin üzerinde birkaç renkli konsol yazısıdır.
    Masaüstünde duvar kâğıdının geçişleri yüzlerce renk üretir, koyu alan yalnızca açık pencerelerdir.
    Piksel piksel karşılaştırma yapılmaz, saat ve tasarım değiştikçe test bozulmasın."""
    def pixel(x, y):
        i = (y * width + x) * 3
        return rgb[i], rgb[i + 1], rgb[i + 2]

    samples = [pixel(x, y) for y in range(0, height, 15) for x in range(0, width, 15)]
    dark = sum(1 for p in samples if max(p) < 24) / len(samples)
    colors = len(set(samples))
    return dark < 0.6 and colors > 200


def save_screenshot(qmp_path, target):
    """Ekran görüntüsünü alır, target .png ise dönüştürür. (genişlik, yükseklik, RGB) döner."""
    ppm = target if target.endswith(".ppm") else target + ".ppm"
    reply = qmp(qmp_path, "screendump", {"filename": os.path.abspath(ppm)})
    if "error" in reply:
        raise RuntimeError(f"Ekran görüntüsü alınamadı: {reply['error']}")
    image = read_ppm(ppm)
    if ppm != target:
        write_png(target, *image)
        os.remove(ppm)
    return image


def send_key(path, combo):
    """QEMU tuş adlarıyla (qcode) tuş kombinasyonu gönderir: "ret", "meta_l-l", "shift-a"."""
    keys = [{"type": "qcode", "data": key} for key in combo.split("-")]
    qmp(path, "send-key", {"keys": keys})
    time.sleep(0.08)


# Fiziksel tuşlar gönderilir, ankaros'ta Türkçe Q düzeni etkindir (XKB_DEFAULT_LAYOUT=tr):
# "." sağ Shift'in solundaki tuş, "/" Shift+7, "-" ve "," Türkçe Q'daki yerleri.
KEY_NAMES = {" ": "spc", ".": "slash", "/": "shift-7", "-": "equal", ",": "backslash"}


def type_text(path, text):
    """Küçük harf, rakam, boşluk ve . / - , karakterlerinden oluşan metni tuş tuş yazar."""
    for char in text:
        send_key(path, KEY_NAMES.get(char, char))


def run_actions(actions, qmp_path, console, log, screenshot):
    """Eylemler ";" ile ayrılır:
    click X Y      sol tıklama (1440x900 koordinatları)
    drag X1 Y1 X2 Y2  sol düğme basılıyken sürükle
    hold X1 Y1 X2 Y2  sürükle ama düğmeyi bırakma
    up             basılı tutulan sol düğmeyi bırak
    move X Y       imleci taşı
    sleep SANIYE   bekle
    key TUŞLAR     tuş kombinasyonu, QEMU adlarıyla: ret, esc, meta_l-l
    keydown TUŞ    tuşu basılı tut (ör. alt), keyup TUŞ ile bırak
    type METİN     küçük harf, rakam ve . / - , yaz (ör. IP adresi)
    run KOMUT      seri konsolda (root) komut çalıştır
    link on|off    sanal ağ kablosunu tak ya da çıkar
    shot DOSYA     o anki ekranın görüntüsünü kaydet (yol --screenshot'a göre)
    """
    for action in filter(None, (a.strip() for a in actions.split(";"))):
        name, _, rest = action.partition(" ")
        values = rest.split()
        if name == "click":
            click(qmp_path, float(values[0]), float(values[1]))
        elif name == "drag":
            drag(qmp_path, *(float(v) for v in values[:4]))
        elif name == "hold":
            drag(qmp_path, *(float(v) for v in values[:4]), release=False)
        elif name == "move":
            move(qmp_path, float(values[0]), float(values[1]))
        elif name == "up":
            button(qmp_path, False)
        elif name == "keydown":
            hold_key(qmp_path, values[0], True)
        elif name == "keyup":
            hold_key(qmp_path, values[0], False)
        elif name == "sleep":
            wait_for(console, b"\x00", float(values[0]), log)
        elif name == "key":
            send_key(qmp_path, values[0])
        elif name == "type":
            type_text(qmp_path, rest)
        elif name == "link":
            qmp(qmp_path, "set_link", {"name": "nic0", "up": values[0] == "on"})
        elif name == "run":
            console.sendall(f"{rest}\n".encode())
        elif name == "shot":
            target = os.path.join(os.path.dirname(os.path.abspath(screenshot)), rest)
            save_screenshot(qmp_path, target)
            print(f"Ekran görüntüsü: {target}", flush=True)
        else:
            raise SystemExit(f"Bilinmeyen eylem: {action}")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--images", default=os.environ.get("IMAGES", default_images()))
    parser.add_argument("--command", default="", help="giriş yapıldıktan sonra arka planda çalıştırılacak komut")
    parser.add_argument("--wait", type=float, default=20,
                        help="ekran görüntüsünden önce beklenecek saniye (--expect-desktop ile: en fazla)")
    parser.add_argument("--expect-desktop", action="store_true",
                        help="masaüstü ekrana gelene kadar bekle, gelmezse başarısız ol")
    parser.add_argument("--actions", default="",
                        help='ör. "click 326 860; sleep 5; key meta_l-l; type ankaros; key ret; shot kilit.png"')
    parser.add_argument("--screenshot", required=True, help="ekran görüntüsünün kaydedileceği .ppm ya da .png")
    parser.add_argument("--log", default=None, help="seri konsol günlüğü (varsayılan: ekran görüntüsünün yanında)")
    parser.add_argument("--boot-timeout", type=float, default=120)
    parser.add_argument("--accel", choices=["hvf", "kvm", "tcg"], help="varsayılan: makineye göre seçilir")
    args = parser.parse_args()

    workdir = tempfile.mkdtemp(prefix="ankaros-qemu-")
    serial_path = os.path.join(workdir, "serial.sock")
    qmp_path = os.path.join(workdir, "qmp.sock")
    log_path = args.log or os.path.splitext(args.screenshot)[0] + ".log"
    # CI'daki arm64 makinelerde KVM yok, işlemci yazılımla taklit edilir (tcg): çalışır ama yavaştır.
    if args.accel:
        accel = args.accel
    elif platform.system() == "Darwin":
        accel = "hvf"
    elif os.access("/dev/kvm", os.R_OK | os.W_OK):
        accel = "kvm"
    else:
        accel = "tcg"
    print(f"Hızlandırma: {accel}", flush=True)

    qemu = subprocess.Popen([
        "qemu-system-aarch64",
        "-M", "virt", "-accel", accel, "-cpu", "host" if accel != "tcg" else "max", "-smp", "4", "-m", "2G",
        "-snapshot",
        "-display", "none",
        "-device", f"virtio-gpu-pci,xres={SCREEN_WIDTH},yres={SCREEN_HEIGHT}",
        "-device", "virtio-keyboard-pci", "-device", "virtio-tablet-pci",
        "-kernel", os.path.join(args.images, "Image"),
        "-append", "rootwait root=/dev/vda console=ttyAMA0",
        "-drive", f"file={os.path.join(args.images, 'rootfs.ext4')},if=none,format=raw,id=hd0,file.locking=off",
        "-device", "virtio-blk-device,drive=hd0",
        "-netdev", "user,id=eth0", "-device", "virtio-net-device,netdev=eth0,id=nic0",
        "-serial", f"unix:{serial_path},server=on,wait=off",
        "-qmp", f"unix:{qmp_path},server=on,wait=off",
    ])

    ok = False
    try:
        for _ in range(50):
            if os.path.exists(serial_path):
                break
            time.sleep(0.1)
        console = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        console.connect(serial_path)
        with open(log_path, "wb") as log:
            if not wait_for(console, b"login:", args.boot_timeout, log):
                print("Giriş istemi gelmedi", file=sys.stderr)
                return 1
            console.sendall(b"root\n")
            wait_for(console, b"# ", 15, log)
            if args.command:
                # Komutun çıktısı da günlüğe düşsün diye seri konsola yönlendirilir.
                console.sendall(f"({args.command}) > /dev/ttyAMA0 2>&1 &\n".encode())
            if args.expect_desktop:
                started = time.time()
                deadline = started + args.wait
                ok = False
                while not ok and time.time() < deadline:
                    wait_for(console, b"\x00", 5, log)
                    ok = looks_like_desktop(*save_screenshot(qmp_path, args.screenshot))
                if ok:
                    print(f"Masaüstü {time.time() - started:.0f} saniyede göründü", flush=True)
                    print_boot_report(console, log)
                else:
                    print(f"Masaüstü {args.wait:.0f} saniyede görünmedi", file=sys.stderr)
            else:
                wait_for(console, b"\x00", args.wait, log)  # süre boyunca konsolu okumaya devam et
                ok = True
            run_actions(args.actions, qmp_path, console, log, args.screenshot)
            save_screenshot(qmp_path, args.screenshot)
            console.sendall(b"echo '--- compositor gunlugu ---'; cat /var/log/ankaros-compositor.log; poweroff\n")
            wait_for(console, b"Power down", 30, log)
    finally:
        # Sanal makine kapandıysa QEMU kendisi çıkar, kapanmadıysa durdurulur.
        try:
            qemu.wait(timeout=10)
        except subprocess.TimeoutExpired:
            qemu.terminate()
            try:
                qemu.wait(timeout=10)
            except subprocess.TimeoutExpired:
                qemu.kill()
    print(f"Ekran görüntüsü: {args.screenshot}\nSeri konsol: {log_path}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
