#!/usr/bin/env python3
"""ankaros imajını QEMU'da ekransız açar, seri konsoldan komut çalıştırır ve ekran görüntüsü alır.

Kullanım:
    ./scripts/qemu-test.py --command "ankaros-shell-preview" --wait 20 --screenshot ekran.ppm
    ./scripts/qemu-test.py --command "true" --wait 15 \
        --actions "click 326 860; sleep 10" --screenshot ekran.ppm

Adımlar: imaj açılır (değişiklikler kaydedilmez, -snapshot), giriş isteminde root olarak girilir,
--command arka planda çalıştırılır, --wait saniye beklenir, --actions sırayla uygulanır (ekran
koordinatları 1440x900'e göre: "click X Y", "sleep SANIYE"), QEMU'nun screendump komutuyla ekran
görüntüsü alınır ve sanal makine kapatılır. Seri konsol çıktısı --log dosyasına yazılır.

GPU 2D sanal karttır (virtio-gpu-pci), Mesa yazılımsal çizer (softpipe). Hız ölçmek için değil,
"ekranda doğru şey görünüyor mu" sorusunu cevaplamak için.
"""

import argparse
import json
import os
import platform
import socket
import subprocess
import sys
import tempfile
import time


def default_images():
    user = os.environ.get("USER", "")
    return os.path.expanduser(f"~/OrbStack/ankaros-build/home/{user}/ankaros-output/images")


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


def click(path, x, y):
    """Sanal tablet üzerinden (mutlak koordinatlı fare) sol tıklama gönderir."""
    def axis(name, value, size):
        return {"type": "abs", "data": {"axis": name, "value": int(value * 32767 / (size - 1))}}

    qmp(path, "input-send-event", {"events": [axis("x", x, SCREEN_WIDTH), axis("y", y, SCREEN_HEIGHT)]})
    time.sleep(0.1)
    for down in (True, False):
        qmp(path, "input-send-event", {"events": [{"type": "btn", "data": {"down": down, "button": "left"}}]})
        time.sleep(0.08)


def run_actions(actions, qmp_path, console, log):
    for action in filter(None, (a.strip() for a in actions.split(";"))):
        name, *values = action.split()
        if name == "click":
            click(qmp_path, float(values[0]), float(values[1]))
        elif name == "sleep":
            wait_for(console, b"\x00", float(values[0]), log)
        else:
            raise SystemExit(f"Bilinmeyen eylem: {action}")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--images", default=os.environ.get("IMAGES", default_images()))
    parser.add_argument("--command", required=True, help="giriş yapıldıktan sonra arka planda çalıştırılacak komut")
    parser.add_argument("--wait", type=float, default=20, help="ekran görüntüsünden önce beklenecek saniye")
    parser.add_argument("--actions", default="", help='ör. "click 326 860; sleep 5" (1440x900 koordinatları)')
    parser.add_argument("--screenshot", required=True, help="ekran görüntüsünün kaydedileceği .ppm dosyası")
    parser.add_argument("--log", default=None, help="seri konsol günlüğü (varsayılan: ekran görüntüsünün yanında)")
    parser.add_argument("--boot-timeout", type=float, default=120)
    args = parser.parse_args()

    workdir = tempfile.mkdtemp(prefix="ankaros-qemu-")
    serial_path = os.path.join(workdir, "serial.sock")
    qmp_path = os.path.join(workdir, "qmp.sock")
    log_path = args.log or os.path.splitext(args.screenshot)[0] + ".log"
    accel = "hvf" if platform.system() == "Darwin" else "kvm" if os.path.exists("/dev/kvm") else "tcg"

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
        "-netdev", "user,id=eth0", "-device", "virtio-net-device,netdev=eth0",
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
            # Komutun çıktısı da günlüğe düşsün diye seri konsola yönlendirilir.
            console.sendall(f"({args.command}) > /dev/ttyAMA0 2>&1 &\n".encode())
            wait_for(console, b"\x00", args.wait, log)  # süre boyunca konsolu okumaya devam et
            run_actions(args.actions, qmp_path, console, log)
            reply = qmp(qmp_path, "screendump", {"filename": os.path.abspath(args.screenshot)})
            if "error" in reply:
                print("Ekran görüntüsü alınamadı:", reply["error"], file=sys.stderr)
                return 1
            console.sendall(b"poweroff\n")
            wait_for(console, b"Power down", 20, log)
            ok = True
    finally:
        qemu.terminate()
        try:
            qemu.wait(timeout=10)
        except subprocess.TimeoutExpired:
            qemu.kill()
    print(f"Ekran görüntüsü: {args.screenshot}\nSeri konsol: {log_path}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
