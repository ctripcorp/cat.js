#!/bin/sh

echo "[build script]start build addon"
node-gyp configure build

echo "[build script]auto generate doc"
apidoc -i src/ -o doc/catjsdoc



expect -c '

#!/usr/bin/expect -f

spawn ssh -p 1022 op1@192.168.83.32 "sudo rm -rf /usr/share/apache-tomcat-7.0.63/webapps/catjsdoc"
#######################
expect {
-re ".*es.*o.*" {
exp_send "yes\r"
exp_continue
}
-re ".*sword.*" {
exp_send "Framework@t.999999\r"
}
}

spawn ssh -p 1022 op1@192.168.83.32 "rm -rf ~/src/catjsdoc"
#######################
expect {
-re ".*es.*o.*" {
exp_send "yes\r"
exp_continue
}
-re ".*sword.*" {
exp_send "Framework@t.999999\r"
}
}

# connect via scp
spawn scp -P 1022 -r doc/catjsdoc "op1@192.168.83.32:~/src" 
#######################
expect {
-re ".*es.*o.*" {
exp_send "yes\r"
exp_continue
}
-re ".*sword.*" {
exp_send "Framework@t.999999\r"
}
}

expect eof

spawn ssh -p 1022 op1@192.168.83.32 "sudo cp -rf ~/src/catjsdoc /usr/share/apache-tomcat-7.0.63/webapps/catjsdoc"
#######################
expect {
-re ".*es.*o.*" {
exp_send "yes\r"
exp_continue
}
-re ".*sword.*" {
exp_send "Framework@t.999999\r"
}
}

interact

'