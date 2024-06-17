#!/bin/bash
echo "Radi se make clean..."
make clean
echo "---------------------------------------------------------------------------"
echo "Radi se make svih fajlova..."
make
echo "---------------------------------------------------------------------------"

if [ "$1" == "brojevi22" ]; then
    echo "Test koji prikazuje rad 3 vm. Svaki gost ima svoje cinjenice o brojevima. Potrebno je uneti broj u konzolu, i zatim pritisnuti enter, i na ekranu ce se prikazati cinjenica o broju. Ako se unese isti broj vise puta, prikazace se razlicite cinjenice"
    ./mini_hypervisor -m 2 -p 2 -g guest1.img guest2.img guest3.img

elif [ "$1" == "brojevi24" ]; then
    echo "Test koji prikazuje rad 3 vm. Svaki gost ima svoje cinjenice o brojevima. Potrebno je uneti broj u konzolu, i zatim pritisnuti enter, i na ekranu ce se prikazati cinjenica o broju. Ako se unese isti broj vise puta, prikazace se razlicite cinjenice"
    ./mini_hypervisor -m 2 -p 4 -g guest1.img guest2.img guest3.img

elif [ "$1" == "brojevi42" ]; then
    echo "Test koji prikazuje rad 3 vm. Svaki gost ima svoje cinjenice o brojevima. Potrebno je uneti broj u konzolu, i zatim pritisnuti enter, i na ekranu ce se prikazati cinjenica o broju. Ako se unese isti broj vise puta, prikazace se razlicite cinjenice"
    ./mini_hypervisor -m 4 -p 2 -g guest1.img guest2.img guest3.img

elif [ "$1" == "brojevi44" ]; then
    echo "Test koji prikazuje rad 3 vm. Svaki gost ima svoje cinjenice o brojevima. Potrebno je uneti broj u konzolu, i zatim pritisnuti enter, i na ekranu ce se prikazati cinjenica o broju. Ako se unese isti broj vise puta, prikazace se razlicite cinjenice"
    ./mini_hypervisor -m 4 -p 4 -g guest1.img guest2.img guest3.img

elif [ "$1" == "brojevi82" ]; then
    echo "Test koji prikazuje rad 3 vm. Svaki gost ima svoje cinjenice o brojevima. Potrebno je uneti broj u konzolu, i zatim pritisnuti enter, i na ekranu ce se prikazati cinjenica o broju. Ako se unese isti broj vise puta, prikazace se razlicite cinjenice"
    ./mini_hypervisor -m 8 -p 2 -g guest1.img guest2.img guest3.img

elif [ "$1" == "brojevi84" ]; then
    echo "Test koji prikazuje rad 3 vm. Svaki gost ima svoje cinjenice o brojevima. Potrebno je uneti broj u konzolu, i zatim pritisnuti enter, i na ekranu ce se prikazati cinjenica o broju. Ako se unese isti broj vise puta, prikazace se razlicite cinjenice"
    ./mini_hypervisor -m 8 -p 4 -g guest1.img guest2.img guest3.img

elif [ "$1" == "konkatenacija22" ]; then
    echo "Test koji prikazuje upis i ispis stringova. Potrebno je uneti string, i zatim pritisnuti enter. Radnja se ponavlja dok se ne unese x, tada se ispisuju svi stringovi, konkatenirani, i program se gasi."
    ./mini_hypervisor -m 2 -p 2 -g guest4.img
elif [ "$1" == "konkatenacija24" ]; then
    echo "Test koji prikazuje upis i ispis stringova. Potrebno je uneti string, i zatim pritisnuti enter. Radnja se ponavlja dok se ne unese x, tada se ispisuju svi stringovi, konkatenirani, i program se gasi."
    ./mini_hypervisor -m 2 -p 4 -g guest4.img
elif [ "$1" == "konkatenacija42" ]; then
    echo "Test koji prikazuje upis i ispis stringova. Potrebno je uneti string, i zatim pritisnuti enter. Radnja se ponavlja dok se ne unese x, tada se ispisuju svi stringovi, konkatenirani, i program se gasi."
    ./mini_hypervisor -m 4 -p 2 -g guest4.img
elif [ "$1" == "konkatenacija44" ]; then
    echo "Test koji prikazuje upis i ispis stringova. Potrebno je uneti string, i zatim pritisnuti enter. Radnja se ponavlja dok se ne unese x, tada se ispisuju svi stringovi, konkatenirani, i program se gasi."
    ./mini_hypervisor -m 4 -p 4 -g guest4.img
elif [ "$1" == "konkatenacija82" ]; then
    echo "Test koji prikazuje upis i ispis stringova. Potrebno je uneti string, i zatim pritisnuti enter. Radnja se ponavlja dok se ne unese x, tada se ispisuju svi stringovi, konkatenirani, i program se gasi."
    ./mini_hypervisor -m 8 -p 2 -g guest4.img
elif [ "$1" == "konkatenacija84" ]; then
    echo "Test koji prikazuje upis i ispis stringova. Potrebno je uneti string, i zatim pritisnuti enter. Radnja se ponavlja dok se ne unese x, tada se ispisuju svi stringovi, konkatenirani, i program se gasi."
    ./mini_hypervisor -m 8 -p 4 -g guest4.img
    
else
    echo "Nepoznat test: $1"
fi


echo "
Treba pocistiti za sobom"
echo "---------------------------------------------------------------------------"
make clean
