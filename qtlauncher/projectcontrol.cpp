
/// Include vl headers
#include "base/filesystem.hpp"

/// Include GUI headers
#include "projectcontrol.h"
#include "mainwindow.h"
#include "projpage.h"

/// Include QT headers
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QMenu>

/// Include standard headers
#include <iostream>


ProjControl::ProjControl( QTreeWidgetItem *treeParent, QWidget *base ) :
        _treeParent( treeParent ),
        _base( base )
{
    std::cout << "ProjControl constructor " << std::endl;

    this->setParent(base);
}


ProjControl::~ProjControl( void )
{
    std::cout << "ProjControl destructor: project name " << _projSet->getCasePtr()->getName() << std::endl;

    if( _projSet->getChanged() )
    {
        QMessageBox msgBox;
        msgBox.setText("Save changes to project " + QString::fromStdString(_projSet->getCasePtr()->getName()) + "?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard );
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        if( ret == 1 )
        {
            std::cerr << "save to file " << _file.toStdString() << std::endl;
            saveToFile(_file);
        }
    }

    for( int u = 0; u < _pages.size(); u++)
    {
        delete _pages.at(u);
    }
    delete _treeItem;
}


bool ProjControl::openProj(QString const &fileStr)
{
    std::cout << "ProjControl openProj: file " << fileStr.toStdString() << std::endl;

    QFileInfo file(fileStr);
    _path = file.path();
    _file = fileStr;

    _projSet.reset( new vl::ProjSettings(fileStr.toStdString()) );
    vl::ProjSettingsSerializer projSer(_projSet);
    //reading file with serializer to settings
    if( projSer.readString( vl::readFileToString( fileStr.toStdString() ) ) )
    {
        createProj();
        return true;
    }
    else
    {
        return false;
    }
    return false;
}


void ProjControl::createProj()
{
    std::cout << "ProjControl createProj " << std::endl;


    ProjPage* projPage = new ProjPage( _projSet, _base, _treeParent, QString::fromStdString(_projSet->getCasePtr()->getName()) );

    connect( projPage, SIGNAL(launch_clicked(QString const &)), this, SLOT(launch_clicked(QString const &)));

    //_projPage->setName(QString::fromStdString(_projSet->getName()));

    _currTreeItem = projPage->getTreeItem();
    _treeItem = projPage->getTreeItem();

    //addScenesAndMaps( QString::fromStdString(_projSet->getName()), _projPage );

    for( unsigned int i = 0; i < _projSet->getNcases(); i++ )
    {
        addCase( QString::fromStdString(_projSet->getCasePtr(i)->getName() ) );
    }
    _projSet->clearChanged();
    _currPage = projPage;
    _pages.append(projPage);

}


void ProjControl::writeProj(QString &fileStr)
{
    std::cout << "ProjControl writeProj:  to file " << fileStr.toStdString() << std::endl;

    std::string file;
    vl::ProjSettingsSerializer projSer(_projSet);
    projSer.writeString( file );
    fileStr = QString::fromStdString(file);
}


bool ProjControl::saveToFile(QString const &fileStr)
{
    std::cout << "ProjControl saveToFile: file " << fileStr.toStdString() << std::endl;

    QString file;
    if(fileStr == "")
    {
        file = _file;
    }
    else
    {
        file = fileStr;
    }
    QString str;
    writeProj(str);
    if( ! vl::writeFileFromString( file.toStdString(), str.toStdString() ) )
    {
        QMessageBox m;
        m.setText("Writing to file " + file + " failed");
        m.exec();
        return false;
    }

    // If project was saved changed bit will be set to false
    if(file == _file)
    {
        _projSet->clearChanged();
    }
    return true;
}


void ProjControl::addCase( QString const &name )
{
    std::cout << "ProjControl addCase: case name " << name.toStdString() << std::endl;

    ProjPage* page = new ProjPage( _projSet, _base, _treeItem, name );

    connect(page, SIGNAL(launch_clicked(QString const &)), this, SLOT(launch_clicked(QString const &)));

  //  page->setName(name);

//    addScenesAndMaps(name, page);

    _pages.append(page);
}


void ProjControl::launch_clicked(QString const &caseName)
{
    std::cout << "ProjControl launch_clicked: case name " << caseName.toStdString() << std::endl;

    if( _projSet->getChanged() )
    {
        QString tmpFile;
        tmpFile = _path.filePath("~" + QString::fromStdString(_projSet->getCasePtr()->getName()) +".prj");

        saveToFile(tmpFile);
        emit launched( tmpFile, caseName );
    }
    else
    {
        emit launched( _file, caseName );
    }
}


void ProjControl::writeSettings(QSettings* settings)
{
    std::cout << "ProjControl writeSettings: project " << _projSet->getCasePtr()->getName() << std::endl;

    settings->beginWriteArray("pages");
    for( int u = 0; u < _pages.size(); u++)
    {
        settings->setArrayIndex(u);
        _pages.at(u)->writeSettings(settings);
    }
    settings->endArray();
}


void ProjControl::readSettings(QSettings* settings)
{
    std::cout << "ProjControl readSettings" << std::endl;

    int size = settings->beginReadArray("pages");
    for (int i = 0; i < size; ++i) {
         settings->setArrayIndex(i);
         _pages.at(i)->readSettings(settings);
    }
    settings->endArray();
}










