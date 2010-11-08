
#include "mainwindow.h"
#include "ui_mainwindow.h"

/// Include standard headers
#include <iostream>
#include <vector>

/// Include QT headers
#include <QMessageBox>
#include <QCheckBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QTreeWidget>
#include <QListWidget>
#include <QDir>
#include <QProcess>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSettings>
#include <QContextMenuEvent>

/// Include vl headers
#include "base/envsettings.hpp"
#include "base/projsettings.hpp"

/// Include GUI headers
#include "envcontrol.h"
#include "projectcontrol.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initializing Environment
//    QTreeWidgetItem *env = new EnvTreeItem(ui->tree->invisibleRootItem());
//    ui->tree->setCurrentItem(env);  // for warning colours needs to be set

    _envCtrl = new EnvControl(ui->tree->invisibleRootItem(), ui->stackBase, ui->stackBaseLayout );

//    ui->tree->resizeColumnToContents(0);

    readSettings();
}

MainWindow::~MainWindow()
{
    writeSettings();
    for( int i = 0; i < _projCtrls.size(); i++ )
    {
        _projCtrls.at(i)->~ProjControl();
    }
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::launched(QString const &file, QString const&name)
{
    QString com = _envCtrl->getCommand();
//
//    QMessageBox m;
//    m.setText("launch " + com);
//    m.exec();

    QStringList arguments;
    arguments << "-e" << _envCtrl->getFile();
    arguments << "-p" << file;
    arguments << "-n" << name;

    QProcess *myProcess = new QProcess(this);
    myProcess->start(com, arguments);
}

void MainWindow::on_actionExit_triggered()
{
    writeSettings();
    QApplication::exit();
}


void MainWindow::on_actionOpenEnvironment_triggered()
{
    // TODO oletuskansio
    QString fileStr = QFileDialog::getOpenFileName(this,
        tr("Open Environment"), "", tr(" (*.env)"));

    if( !fileStr.size() == 0 )
    {
        _envCtrl->openEnv(fileStr);
    }
}


void MainWindow::on_actionOpenProject_triggered()
{
    // TODO oletuskansio
    QString fileStr = QFileDialog::getOpenFileName(this,
        tr("Open Project"), "", tr(" (*.prj)"));

    if( !fileStr.size() == 0 )
    {
        createProjCtrl(fileStr);
    }
}



void MainWindow::writeSettings()
{
    QSettings* settings = new QSettings("VRLab", "Configurator");

    settings->beginGroup("MainWindow");
    settings->setValue("size", size());
    settings->setValue("pos", pos());
    settings->endGroup();

    settings->beginGroup("envCtrl");
    _envCtrl->writeSettings(settings);
    settings->endGroup();


    settings->beginWriteArray("projCtrls");
    for( int c = 0; c < _projCtrls.size(); c++ )
    {
        std::cerr << " write projs" << std::endl;
        settings->setArrayIndex(c);
        _projCtrls.at(c)->writeSettings(settings);
        settings->setValue("file", _projCtrls.at(c)->getFile());
    }
     settings->endArray();

     delete settings;
}


void MainWindow::readSettings()
{
    QSettings* settings = new QSettings("VRLab", "Configurator");

    settings->beginGroup("MainWindow");
    resize(settings->value("size", QSize(601, 451)).toSize());
    move(settings->value("pos", QPoint(200, 200)).toPoint());
    settings->endGroup();

    settings->beginGroup("envCtrl");
    QString env = settings->value("file", "").toString();
    QFileInfo envFile(env);
    if( env != "" )
    {
        if(envFile.exists())
        {
            if( !_envCtrl->openEnv(env) )
            {
                // TODO error in envserializer
                std::cerr << "env ei ole olemassa" << std::endl;
            }
        }
        else
        {
            // TODO Print error
        }
    }
    _envCtrl->readSettings(settings);
    settings->endGroup();


    int size = settings->beginReadArray("projCtrls");
    for (int c = 0; c < size; ++ c)
    {
        settings->setArrayIndex(c);
        QFileInfo file(settings->value("file").toString());
        if( file.exists() )
        {
            createProjCtrl(settings->value("file").toString());
            _projCtrls.last()->readSettings(settings);
        }
        else
        {
            // TODO virheilmo: ei lytynyt file
        }
    }
    settings->endArray();

    delete settings;
}


void MainWindow::createProjCtrl(QString const &file)
{
    for( int i = 0; i < _projCtrls.size(); i++ )
    {
        std::cerr << "kn " << file.toStdString() << std::endl;
        if( _projCtrls.at(i)->getFile() == file )
        {
            QMessageBox m;
            m.setText("Project " + file + " is already open");
            m.exec();
            return;
        }
    }

    ProjControl* ctrl = new ProjControl(ui->tree->invisibleRootItem(), ui->stackBase );
    if( ctrl->openProj(file) )
    {
        //ctrl->update();
        connect(ctrl, SIGNAL(launched(QString const &, QString const &)), this, SLOT(launched(QString const &, QString const &)));
        _projCtrls << ctrl;
    }
    else
    {
        // TODO unknown error in projsettings eg. errenous xml
        delete ctrl;
    }
}


void MainWindow::on_actionClose_Project_triggered()
{
   // ProjTreeItem* item = dynamic_cast<ProjTreeItem *>( _currFocusTreeItem );

    QTreeWidgetItem* item = _currFocusTreeItem;

    for( int c = 0; c < _projCtrls.size(); c++ )
    {
        if(_projCtrls.at(c)->getProjTreeItem() == item)
        {
            delete _projCtrls.at(c);
            _projCtrls.erase(_projCtrls.begin() + c);
        }
    }

}

bool MainWindow::isProjectTreeItem(QTreeWidgetItem* item)
{
    for( int i = 0; i < _projCtrls.size(); i++ )
    {
        if( _projCtrls.at(i)->getProjTreeItem()->text(0) == item->text(0) )
        {
            return true;
        }
    }
    return false;
}

void MainWindow::on_tree_customContextMenuRequested(QPoint pos)
{
    _currFocusTreeItem = ui->tree->itemAt(pos.x(), pos.y());

    if( _currFocusTreeItem )
    {
        if( isProjectTreeItem(_currFocusTreeItem) )
        {

                QMenu menu(ui->tree);
                menu.addAction(ui->actionClose_Project);
                menu.exec(ui->tree->mapToGlobal( pos ));
        }
    }
    else
    {
        QMenu menu(ui->tree);
        menu.addAction(ui->actionOpenProject);
        menu.exec(ui->tree->mapToGlobal( pos ));

    }
}

void MainWindow::on_actionSave_triggered()
{
//    QString xml;
    for( int i = 0; i < _projCtrls.size(); i++ )
    {
        _projCtrls.at(i)->saveToFile(); //->writeProj(xml);
//        ::writeFileFromString( _projCtrls.at(i)->getFile().toStdString(), xml.toStdString() );
//        xml.clear();
    }
}
