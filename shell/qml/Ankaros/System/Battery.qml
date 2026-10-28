// Mac önizlemesi için sahte pil (Ankaros.System). ankaros'ta bunun yerine C++ Battery sınıfı
// (battery.cpp) /sys/class/power_supply'dan okur. Sanal makinelerde olduğu gibi pil yok sayılır.
pragma Singleton

import QtQuick

QtObject {
    readonly property bool present: false
    readonly property int level: 0
    readonly property bool charging: false
}
