# ankaros imajını Windows'ta QEMU ile açar (derleme WSL2'de yapılır, bkz. docs/gelistirme-ortami.md).
#
#   .\scripts\run-qemu.ps1                  bilgisayarın mimarisindeki imaj (Intel/AMD'de x86_64), WHPX ile
#                                           donanım hızlandırmalı
#   .\scripts\run-qemu.ps1 -Arch aarch64    aarch64 imajı: x86 bilgisayarda yazılımla taklit (tcg), çok yavaş
#   .\scripts\run-qemu.ps1 -NoGui           yalnızca seri konsol
#   .\scripts\run-qemu.ps1 -Snapshot        imajda ve veri diskinde yapılan değişiklikleri kapanışta atar
#   .\scripts\run-qemu.ps1 -Refresh         imajları WSL'deki derlemeden yeniden kopyalar
#   .\scripts\run-qemu.ps1 -NoGl            virgl (ana bilgisayarın GPU'su) yerine yazılımla çizim (çok yavaş)
#   .\scripts\run-qemu.ps1 -Width 1900 -Height 960   ekran çözünürlüğü (varsayılan 1440x900). 1920x1080'lik monitörde
#                                           pencere başlık çubuğuyla sığsın diye 1900x960 gibi. Genişlik 2560'ı
#                                           aşarsa ankaros arayüzü 2 kat ölçekle çizer.
#   .\scripts\run-qemu.ps1 -Audio none     ses çıkışını kapatır (varsayılan dsound: Windows ses aygıtı)
#   .\scripts\run-qemu.ps1 -Fit            pencereyi ekrana sığacak şekilde ölçekler (görüntü bulanıklaşır)
#
# WHPX için Windows'ta "Windows Hypervisor Platform" özelliği açık olmalı, açık değilse -Accel tcg verin.
# İmajlar WSL'den $Images klasörüne kopyalanır: QEMU, WSL'in ağ dosya sisteminde bayt kilidi hatası verir.
param(
  [ValidateSet('aarch64', 'x86_64')][string]$Arch = $(if ($env:PROCESSOR_ARCHITECTURE -eq 'ARM64') { 'aarch64' } else { 'x86_64' }),
  [string]$Images,
  [string]$WslDistro = 'Ubuntu',
  [string]$WslUser,
  [string]$Qemu,
  [string]$Accel,
  [string]$Cpu,
  [string]$Data,
  [string]$Audio = 'dsound',
  [int]$Width = 1440,
  [int]$Height = 900,
  [switch]$NoGui,
  [switch]$NoGl,
  [switch]$Fit,
  [switch]$Snapshot,
  [switch]$Refresh,
  [Parameter(ValueFromRemainingArguments)][string[]]$Extra
)
$ErrorActionPreference = 'Stop'

# scripts/build.sh çıktıyı bilgisayarın kendi mimarisi için ~/ankaros-output'a, başka mimari için
# ~/ankaros-output-MİMARİ'ye yazar (ANKAROS_OUT ile başka bir klasör verilmişse ikincisi de aranır)
$hostArch = if ($env:PROCESSOR_ARCHITECTURE -eq 'ARM64') { 'aarch64' } else { 'x86_64' }
$outDirs = if ($Arch -eq $hostArch) { @('ankaros-output', "ankaros-output-$Arch") } else { @("ankaros-output-$Arch") }
$kernel = if ($Arch -eq 'x86_64') { 'bzImage' } else { 'Image' }
if (-not $Images) { $Images = "$env:USERPROFILE\ankaros-images-$Arch" }
if (-not $Qemu) { $Qemu = "C:\Program Files\qemu\qemu-system-$Arch.exe" }
if (-not $Accel) { $Accel = if ($Arch -eq 'x86_64') { 'whpx' } else { 'tcg' } }

if ($Refresh -or -not (Test-Path "$Images\$kernel") -or -not (Test-Path "$Images\rootfs.ext4")) {
  # Derlemenin yapıldığı WSL kullanıcısı, Windows kullanıcısından farklı olabilir
  if (-not $WslUser) { $WslUser = (wsl.exe -d $WslDistro --exec whoami).Trim() }
  $wslHome = "\\wsl.localhost\$WslDistro\home\$WslUser"
  $outDir = $outDirs | Where-Object { Test-Path "$wslHome\$_\images\$kernel" } | Select-Object -First 1
  if (-not $outDir) { throw "$Arch imajı bulunamadı: $wslHome\$($outDirs -join ', ')\images (önce WSL'de derleyin)" }
  $wsl = "$wslHome\$outDir\images"
  New-Item -ItemType Directory -Force $Images | Out-Null
  Copy-Item "$wsl\$kernel" "$Images\$kernel" -Force
  # rootfs.ext4 WSL'de rootfs.ext2'ye bir bağlantı, ağ yolundan doğrudan kopyalanamaz
  Copy-Item "$wsl\rootfs.ext2" "$Images\rootfs.ext4" -Force
}

if ($Arch -eq 'x86_64') {
  # -vga none: q35'in varsayılan standart VGA'sı kalırsa Mesa onu seçer ve sanal GPU'yu kullanmadan
  # yazılımla çizer (softpipe)
  $machine = @('-M', 'q35', '-vga', 'none')
  $console = 'ttyS0'; $block = 'virtio-blk-pci'; $net = 'virtio-net-pci'
  if (-not $Cpu) { $Cpu = if ($Accel -eq 'tcg') { 'max' } else { 'qemu64' } }
  $accelArg = if ($Accel -eq 'whpx') { 'whpx,kernel-irqchip=off' } else { $Accel }
} else {
  $machine = @('-M', 'virt')
  $console = 'ttyAMA0'; $block = 'virtio-blk-device'; $net = 'virtio-net-device'
  if (-not $Cpu) { $Cpu = 'max' }
  $accelArg = if ($Accel -eq 'tcg') { 'tcg,thread=multi' } else { $Accel }
}

$display = if ($NoGui) {
  @('-nographic')
} else {
  # virgl (virtio-gpu-gl) yalnızca x86_64'te denendi
  $gl = (-not $NoGl) -and $Arch -eq 'x86_64'
  $glState = if ($gl) { 'on' } else { 'off' }
  # zoom-to-fit kapalı: pencere ekranla aynı boyutta (1:1) gösterilir, ölçeklenince görüntü bulanıklaşır
  $zoomState = if ($Fit) { 'on' } else { 'off' }
  $gpu = if ($gl) { 'virtio-gpu-gl-pci' } else { 'virtio-gpu-pci' }
  @('-display', "gtk,zoom-to-fit=$zoomState,show-cursor=on,gl=$glState", '-serial', 'mon:stdio',
    '-device', "$gpu,xres=$Width,yres=$Height",
    '-device', 'virtio-keyboard-pci', '-device', 'virtio-tablet-pci')
}

# Veri diski: ilk açılışta ankaros onu biçimlendirip /var/lib/ankaros'a bağlar (karar 0017)
if (-not $Data) { $Data = "$env:LOCALAPPDATA\ankaros\veri.qcow2" }
if (-not (Test-Path $Data)) {
  New-Item -ItemType Directory -Force (Split-Path $Data) | Out-Null
  & (Join-Path (Split-Path $Qemu) 'qemu-img.exe') create -q -f qcow2 $Data 16G
}

# x86_64: ekran modunu imaj ankaros.mode ile okur (/etc/profile.d/ankaros-grafik.sh); aarch64'te UTM'nin xres/yres'i geçerli
$append = "rootwait root=/dev/vda console=$console"
if ($Arch -eq 'x86_64') { $append += " ankaros.mode=${Width}x${Height}" }

$qargs = $machine + @('-accel', $accelArg, '-cpu', $Cpu, '-smp', '4', '-m', '2G') + $display + @(
  '-kernel', "$Images\$kernel",
  '-append', $append,
  '-drive', "file=$Images\rootfs.ext4,if=none,format=raw,id=hd0",
  '-device', "$block,drive=hd0",
  '-drive', "file=$Data,if=none,format=qcow2,id=veri",
  '-device', 'virtio-blk-pci,drive=veri',
  '-audiodev', "$Audio,id=ses", '-device', 'virtio-sound-pci,audiodev=ses',
  '-netdev', 'user,id=eth0', '-device', "$net,netdev=eth0"
)
if ($Snapshot) { $qargs += '-snapshot' }
if ($Extra) { $qargs += $Extra }

& $Qemu @qargs
