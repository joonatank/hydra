
/// Include QT headers
#include <QtGui/QApplication>

/// Include standard headers
#include <iostream>

/// Include GUI headers
#include "mainwindow.h"

/// Include vl headers
#include "base/exceptions.hpp"

int main(int argc, char *argv[])
{
    try
    {
        QApplication a(argc, argv);
        MainWindow w;
        w.show();
        return a.exec();
    }
    catch( vl::exception const &e )
    {
        std::cerr << "vl::exception" << boost::diagnostic_information<>(e) << std::endl;
    }
    catch( std::exception const &e )
    {
        std::cerr << "std exception : " << e.what() << std::endl;
    }
}
