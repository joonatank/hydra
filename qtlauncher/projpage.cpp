
/// Include QT headers
#include <QTreeWidgetItem>
#include <QDir>
#include <QSettings>
#include <QVBoxLayout>
#include <QSignalMapper>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QMenu>
#include <QStringList>

/// Include standard headers
#include <iostream>

/// Include GUI headers
#include "ui_projpage.h"
#include "projpage.h"

/// Include vl headers
#include "base/projsettings.hpp"


const QStringList MAP_FILES = ( QStringList() << "*.map");
const QStringList SCENE_FILES = ( QStringList() << "*.scene");



ProjPage::ProjPage( vl::ProjSettingsRefPtr projSet, QWidget* base, QTreeWidgetItem* treeParent, QString const & name ) :
        ui( new Ui::ProjPage() ),
        _projSet( projSet )
{

    std::cout << "ProjPage constructor: name " << name.toStdString() << "." << std::endl;

    _treeItem = new QTreeWidgetItem(treeParent);

    _treeItem->setText(0,name);
    ui->setupUi(this);
    this->setParent(base);

    ui->name->setText(name);
    ui->launch->setText("Launch " + name);

    update();

    this->hide();

    connect(treeParent->treeWidget(), SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(changePage(QTreeWidgetItem*)));

}

void ProjPage::update()
{
    std::cout << "ProjPage update: name  " << ui->name->text().toStdString() << std::endl;

    _case = _projSet->getCasePtr( ui->name->text().toStdString() );

    for( unsigned int i = 0; i < _case->getNscenes(); i++ )
    {
        vl::ProjSettings::Scene* s = _case->getScenePtr(i);
        QFileInfo file( QString::fromStdString( s->getFile() ) );
        addScene( file,
                  QString::fromStdString( s->getName() ),
                  s->getUse(),
                  QString::fromStdString( s->getAttachtoScene() ),
                  QString::fromStdString( s->getAttachtoPoint() )  );
    }

    for( unsigned int u = 0; u < _case->getNmaps(); u++ )
    {
        vl::ProjSettings::ActionMap* m = _case->getMapPtr(u);
        QFileInfo file( QString::fromStdString( m->getFile() ) );
        addMap( file, m->getUse() );
    }
    
    for( unsigned int a = 0; a < _case->getNscripts(); a++ )
    {
        vl::ProjSettings::Script* m = _case->getScriptPtr(a);
        QFileInfo file( QString::fromStdString( m->getFile() ) );
        addScript( file, m->getUse() );
    }

    updateMapsCombo();
}

ProjPage::~ProjPage( void )
{
    std::cout << "ProjPage destructor: name  " << ui->name->text().toStdString() << std::endl;

}


void ProjPage::changePage(QTreeWidgetItem* item)
{
    std::cout << "ProjPage changePage: item name  " << item->text(0).toStdString() << std::endl;

    if( item == _treeItem )
    {
        this->show();
    }
    else
        this->hide();
}


void ProjPage::addScene( QFileInfo const &file,
                         QString const &name,
                         bool set,
                         QString const &attachto_scene,
                         QString const &attachto_point )
{

    std::cout << "ProjPage addScene: page name  " << ui->name->text().toStdString() << " : scene name " << name.toStdString() << std::endl;

    QWidget* cell = new QWidget(ui->sceneScrollWidget);
    QVBoxLayout* cellLayout = new QVBoxLayout(cell);
    QCheckBox* but = new QCheckBox(cell);
    QLabel* path = new QLabel(cell);
    if( name == "" )
    {
        but->setText( file.baseName() );
    }
    else
    {
        but->setText( name );
    }
    but->setChecked( set );
    _scenes.append( but );

    path->setText( file.fileName() );

    bool foundInFile = existsIn("resources", path->text());

    if( !foundInFile )
    {
        cell->setDisabled(true);
    }

    QWidget* combos = new QWidget(cell);
    QHBoxLayout* combosLayout = new QHBoxLayout(combos);
    QComboBox* add2scene = new QComboBox;
    combosLayout->addWidget(add2scene);
    QComboBox* add2point = new QComboBox;
    combosLayout->addWidget(add2point);

    combos->setLayout(combosLayout);

    cellLayout->addWidget(but);
    cellLayout->addWidget(path);
    cellLayout->addWidget(combos);
    cell->setLayout(cellLayout);

    ui->sceneScrollLayout->addWidget(cell);
}


void ProjPage::addMap( QFileInfo const &file, bool set )
{
	std::cout << "ProjPage addScene: page name  "
		<< ui->name->text().toStdString() << " : map file "
		<< file.filePath().toStdString() << std::endl;

    QCheckBox* but = new QCheckBox( ui->mapScrollWidget );

    but->setText( file.fileName() );

	but->setChecked( set );

    bool foundInFile = existsIn("actionMaps", but->text());

    if( !foundInFile )
    {
        but->setDisabled(true);
    }

    QSignalMapper* signalMapper = new QSignalMapper(this);

    connect(but, SIGNAL(toggled(bool)), signalMapper, SLOT(map()));
    signalMapper->setMapping(but, but);
    connect(signalMapper, SIGNAL(mapped(QWidget*)), this, SLOT(map_toggled(QWidget*)));

    QSignalMapper* signalMapper2 = new QSignalMapper(this);

    QAction* rmAct = new QAction("Remove Map", but);
    but->addAction(rmAct);
    connect(rmAct, SIGNAL(triggered()), signalMapper2, SLOT(map()));
    signalMapper2->setMapping(rmAct, but);
    connect(signalMapper2, SIGNAL(mapped(QWidget*)), this, SLOT(rmMap(QWidget*)));

    ui->mapScrollLayout->addWidget(but);
}

void ProjPage::addScript( QFileInfo const &file, bool set )
{
    std::cout << "ProjPage addScript: page name  " << ui->name->text().toStdString() << " : script file " << file.filePath().toStdString() << std::endl;

    QCheckBox* but = new QCheckBox( ui->scriptScrollWidget );

    but->setText( file.fileName() );

   but->setChecked( set );

    bool foundInFile = existsIn("scripts", but->text());

    if( !foundInFile )
    {
        but->setDisabled(true);
    }

    QSignalMapper* signalMapper = new QSignalMapper(this);

    connect(but, SIGNAL(toggled(bool)), signalMapper, SLOT(map()));
    signalMapper->setMapping(but, but);
    connect(signalMapper, SIGNAL(mapped(QWidget*)), this, SLOT(script_toggled(QWidget*)));

    QSignalMapper* signalMapper2 = new QSignalMapper(this);

    QAction* rmScr = new QAction("Remove Script", but);
    but->addAction(rmScr);
    connect(rmScr, SIGNAL(triggered()), signalMapper2, SLOT(map()));
    signalMapper2->setMapping(rmScr, but);
    connect(signalMapper2, SIGNAL(mapped(QWidget*)), this, SLOT(rmScript(QWidget*)));

    ui->scriptScrollLayout->addWidget(but);
}

 void ProjPage::setName( QString const &name )
 {
     std::cout << "ProjPage addScene: page name  " << ui->name->text().toStdString() << " : new name " << name.toStdString() << std::endl;

     ui->name->setText(name);
     ui->launch->setText( "Launch " + name );
     _treeItem->setText(0,name);
 }


void ProjPage::map_toggled(QWidget* but)
{
    std::cout << "ProjPage map_toggle " << std::endl;

    QCheckBox* box = dynamic_cast<QCheckBox*>( but );
    if( !box )
    {
        std::cerr << "ret " << std::endl;
        return;
    }
    _case->getMapPtr( box->text().toStdString() )->mapOnOff( box->isChecked() );
}

void ProjPage::script_toggled(QWidget* but)
{
    std::cout << "ProjPage script_toggle " << std::endl;

    QCheckBox* box = dynamic_cast<QCheckBox*>( but );
    if( !box )
    {
        std::cerr << "ret " << std::endl;
        return;
    }
    _case->getScriptPtr( box->text().toStdString() )->scriptOnOff( box->isChecked() );
}


void ProjPage::on_launch_clicked()
{
    emit launch_clicked( ui->name->text() );
}


void ProjPage::writeSettings(QSettings* settings)
{
    std::cout << "ProjPage writeSettings " << std::endl;

    settings->setValue("expanded", _treeItem->isExpanded());
    settings->setValue("hidden", this->isHidden());
    settings->setValue("treeItemSelected", _treeItem->isSelected() );
}

void ProjPage::readSettings(QSettings* settings)
{
        std::cout << "ProjPage readSettings " << std::endl;

        _treeItem->setExpanded(settings->value("Expanded", false).toBool());
        if(settings->value("hidden", true).toBool())
        {
            this->hide();
        }
        else
        {
            this->show();
        }
        _treeItem->setSelected(settings->value("treeItemSelected", false).toBool());
}

bool ProjPage::existsIn(QString const &folder, QString const &item)
{
    std::cout << "ProjPage existsIn " << std::endl;

    QFileInfoList listFromFolder = getDir(folder);

    // checking does map appear in a resources folder
    for( int i = 0; i < listFromFolder.size(); i++ )
    {
        if( item == listFromFolder.at(i).fileName() )
        {
            return true;
        }
    }
    return false;
}


QFileInfoList ProjPage::getDir(QString const & folderName)
{
    std::cout << "ProjPage getDir " << std::endl;

    QFileInfo file = QString::fromStdString(_projSet->getFile());
    QDir dir = file.dir();
    dir.cd(folderName);

    if( folderName == "resources")
    {
        dir.setNameFilters(SCENE_FILES);
    }
    else if( folderName == "actionMaps")
    {
        dir.setNameFilters(MAP_FILES);
    }
    return QFileInfoList(dir.entryInfoList(QDir::Files, QDir::Name));
}


void ProjPage::on_mapScrollWidget_customContextMenuRequested(QPoint pos)
{
    QCheckBox* box = dynamic_cast<QCheckBox*>( ui->mapScrollWidget->childAt(pos) );
    if( !box )
    {
        return;
    }

    QMenu menu(ui->sceneScrollWidget);

    for( int i = 0; i < box->actions().size(); i++ )
    {
        menu.addAction( box->actions().at(i) );
    }
    //map->actions().size()
    menu.exec( ui->mapScrollWidget->mapToGlobal(pos) );
}


void ProjPage::rmMap(QWidget* but)
{
    std::cerr << "ProjPage rmMap " << std::endl;

    QCheckBox* box = dynamic_cast<QCheckBox*>( but );
    if( !box )
    {
        return;
    }

    _case->rmMap( box->text().toStdString() );
    delete box;
}

void ProjPage::rmScript(QWidget* but)
{
    std::cerr << "ProjPage rmScript " << std::endl;

    QCheckBox* box = dynamic_cast<QCheckBox*>( but );
    if( !box )
    {
        return;
    }

    _case->rmScript( box->text().toStdString() );
    delete box;
}

void ProjPage::updateMapsCombo()
{
    QFileInfoList listFromFolder = getDir("actionMaps");
    QStringList maps;
    maps << "Add map";
    for( int i = 0; i < listFromFolder.size(); i++ )
    {
        maps << listFromFolder.at(i).fileName();
    }
    ui->mapsCombo->clear();

    ui->mapsCombo->addItems(maps);
}

void ProjPage::on_mapsCombo_activated(QString map)
{
    addMap(map);
}

void ProjPage::on_scriptsScrollArea_customContextMenuRequested(QPoint pos)
{
    QCheckBox* box = dynamic_cast<QCheckBox*>( ui->scriptScrollWidget->childAt(pos) );
    if( !box )
    {
        return;
    }

    QMenu menu(ui->scriptScrollWidget);

    for( int i = 0; i < box->actions().size(); i++ )
    {
        menu.addAction( box->actions().at(i) );
    }
    //map->actions().size()
    menu.exec( ui->scriptScrollWidget->mapToGlobal(pos) );
}
