
#include <stdio.h>
#include <iostream>
#include <QMessageBox>
#include <QApplication>

int main(int argc, char *argv[])
{

   std::string out = "";

    for( int i = 0; i < argc; i++ )
    {
        out = out + argv[i] + " ";
        //std::cerr << argv[i] << " ";
    }

    //std::cerr << std::out << std::endl << "ohjelma kynnisty" << std::endl;

    QApplication a(argc, argv);
    QMessageBox w;
    w.setText( QString::fromStdString(out + " " + "ohjelma kynnisty"));
    w.show();
    return a.exec();
}
