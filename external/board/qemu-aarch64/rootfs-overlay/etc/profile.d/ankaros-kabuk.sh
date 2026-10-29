# Etkileşimli kabuk: istem "kullanıcı@makine:klasör$ ", kullanıcı adı vurgu renginde (Terminal ve
# seri konsol). Renkler 24 bit, Terminal ve modern terminaller destekler.
PS1='\[\e[38;2;192;103;78m\]\u@\h\[\e[0m\]:\w\$ '
# Terminal'de (xterm uyumlu) pencere ve sekme başlığı da istemle ayarlanır: "kullanıcı@makine: klasör".
# Terminal klasörü kabuğun /proc/<pid>/cwd'sinden okuyamıyor: /bin/sh Busybox, Busybox setuid kurulu ve
# böyle süreçlerin klasörünü aynı kullanıcı bile göremiyor. Linux konsolunda bu dizi bozuk görünür.
case "$TERM" in
  xterm*) PS1='\[\e]0;\u@\h: \w\a\]'"$PS1" ;;
esac
alias ls='ls --color=auto'
