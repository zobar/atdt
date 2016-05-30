package require ssh

proc Connect {bind session} {
    puts "Connect $bind $session"
    $session handleKeyExchange
    $session configure -blocking false
    $session destroy
}

set bind [ssh::bind new -port 2222 -rsakey tmp/id_rsa]
$bind configure -connect [list Connect $bind]
$bind listen

after 3000 {set done true}
vwait done

$bind destroy
