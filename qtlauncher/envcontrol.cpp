
/// Include vl headers
#include "base/filesystem.hpp"

/// Include GUI headers
#include "envcontrol.h"
#include "ui_envcontrol.h"

/// Include QT headers
#include <QCheckBox>
#include <QRadioButton>



const QStringList PLUGIN_FILES = ( QStringList() << "*.so");
const QStringList EQC_FILES = ( QStringList() << "*.eqc");
const QStringList TRACKING_FILES = ( QStringList() << "*.trc");



EnvTreeItem::EnvTreeItem( QTreeWidgetItem *parent )
    : QTreeWidgetItem( parent )
{
    setText(0, "Environment");

    plugins = new QTreeWidgetItem(this);
    plugins->setText(0, "Plugins");

    eqc = new QTreeWidgetItem(this);
    eqc->setText(0, "Equalizer");

    tracking = new QTreeWidgetItem(this);
    tracking->setText(0, "Tracking");
}





EnvControl::EnvControl( QTreeWidgetItem *treeParent, QWidget *stackBase, QVBoxLayout* a) :
        ui( new Ui::EnvControl() ),
        _envSet( ),
        _treeItem( new EnvTreeItem( treeParent ) )
{
    ui->setupUi(this);
    this->setParent( stackBase );
    a->addWidget(this);


    _pageMap.insert( _treeItem, ui->pageEnv );
    _pageMap.insert( _treeItem->plugins, ui->pagePlugins );
    _pageMap.insert( _treeItem->eqc, ui->pageEqc );
    _pageMap.insert( _treeItem->tracking, ui->pageTracking );

    connect(treeParent->treeWidget(), SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(changePage(QTreeWidgetItem*)));
}

EnvControl::~EnvControl( void )
{
    delete ui;
    delete _treeItem;
}

void EnvControl::changePage(QTreeWidgetItem* item)
{
    if( _pageMap.contains(item) )
    {
        ui->stack->setCurrentWidget( _pageMap.value(item) );
        //ui->stack->show();
        this->show();
    }
    else
    {
        //ui->stack->hide();
        this->hide();
    }
}


bool EnvControl::openEnv(QString const &path)
{

    ui->pathEnv->setText(path);
    _file = path;

    _envSet.reset( new vl::EnvSettings );
    vl::EnvSettingsSerializer envSer(_envSet);
    //reading file with serializer to settings
    if( envSer.readString( vl::readFileToString( path.toStdString() ) ) )
    {
        updateEnv();
        return true;
    }
    else
        return false;
}


void EnvControl::updateEnv()
{
    // Setting dir to environment file
    QFileInfo file(ui->pathEnv->text());
    QDir dir(file.path());

    dir.cd("plugins");
    updatePluginsTable(dir);

    dir.cdUp();
    dir.cd("tracking");
    updateTrackingTable(dir);

    dir.cdUp();
    dir.cd("eqc");
    updateEqcTable(dir);
}

void EnvControl::updatePluginsTable(QDir dir)
{
    //QVBoxLayout* layout = new QVBoxLayout(ui->pagePlugins);
    //QButtonGroup* buttonGroup = new QButtonGroup(ui->tablePlugins);

    //ui->buttonGroup->setExclusive(false);
//    buttonGroup->setExclusive(false);

    dir.setNameFilters(PLUGIN_FILES);
    QFileInfoList listFromFolder(dir.entryInfoList(QDir::Files, QDir::Name));

    for( unsigned int i = 0; i < _envSet->getPlugins().size(); i++ )
    {

        QCheckBox* but = new QCheckBox(ui->tablePlugins);

        QString name = QString::fromStdString(_envSet->getPlugins().at(i).first);
        but->setText(name);

        if( _envSet->getPlugins().at(i).second == false )
        { but->setCheckState(Qt::Unchecked); }
        else
        { but->setCheckState(Qt::Checked); }

        // Checking through plugins file to see that all defined plugins are found
        // If plugin is missing it will be disabled
        bool found = false;
        for( int u = 0; u < listFromFolder.size(); u++ )
        {
            if(  name == listFromFolder.at(u).baseName() )
            {
                found = true;
            }
        }
        if(!found)
        {
            but->setDisabled(true);
        }

        //ui->buttonGroup->addButton(but);

//        ui->buttonGroupPlugins->addButton(but);
        ui->tablePluginsLayout->addWidget(but);
        //layout->addWidget(but);
    }

    // ui->tablePlugins->setLayout(layout);
}



void EnvControl::updateTrackingTable(QDir dir)
{
//    QVBoxLayout* layout = new QVBoxLayout(ui->pageTracking);
//    QButtonGroup* buttonGroup = new QButtonGroup(ui->tableTracking);
//    buttonGroup->setExclusive(false);

    // listFromFolder consist of all files from the [en file path]/[folder]
    dir.setNameFilters(TRACKING_FILES);
    QFileInfoList listFromFolder(dir.entryInfoList(QDir::Files, QDir::Name));

    // string list of all active items from the envSet
    QList<QString> listOfActive;
    for( unsigned int i = 0; i < _envSet->getTracking().size(); i++ )
    {
        listOfActive << QString::fromStdString(_envSet->getTracking().at(i));
    }

    // Making list from all items from the folder
    for( int u = 0; u < listFromFolder.size(); u++ )
    {
        QCheckBox* but = new QCheckBox(ui->tableTracking);
        QString baseName = listFromFolder.at(u).baseName();
        QString fileName = listFromFolder.at(u).fileName();

        but->setText( baseName );

        if( listOfActive.indexOf(fileName) < 0 )
        {
            but->setCheckState(Qt::Unchecked);
        }
        else
        {
            but->setCheckState(Qt::Checked);
            //removing found item -> possibility to check if there is some plugins not in the folder
            listOfActive.erase(listOfActive.begin()+listOfActive.indexOf(fileName));
        }
//        u->addButton(but);
        ui->tableTrackingLayout->addWidget(but);
    }

    //Adding to list not found items greyed
    for( int l = 0; l < listOfActive.size(); l++ )
    {
        QCheckBox* but = new QCheckBox(ui->tableTracking);
        QString name = QFileInfo(listOfActive.at(l)).baseName();
        but->setText( name );
        but->setCheckState(Qt::Checked);
        but->setDisabled(true);

        //buttonGroup->addButton(but);
        ui->tableTrackingLayout->addWidget(but);
    }
    //ui->tableTracking->setLayout(layout);
}



void EnvControl::updateEqcTable(QDir dir)
{
//    QVBoxLayout* layout = new QVBoxLayout(ui->pageEqc);
//    QButtonGroup* buttonGroup = new QButtonGroup(ui->tableEqc);
//    buttonGroup->setExclusive(true);

    // listFromFolder consist of all files from the [en file path]/[folder]
    dir.setNameFilters(EQC_FILES);
    QFileInfoList listFromFolder(dir.entryInfoList(QDir::Files, QDir::Name));

    // Active one
    QString active = QString::fromStdString(_envSet->getEqc());
    bool found = false;

    // Making list from all items from the folder
    for( int u = 0; u < listFromFolder.size(); u++ )
    {
        QRadioButton* but = new QRadioButton(ui->tableEqc);
        QString baseName = listFromFolder.at(u).baseName();
        QString fileName = listFromFolder.at(u).fileName();

        but->setText( baseName );

        if( fileName == active )
        {
            but->setDown(true);
            found = true;
        }
//        buttonGroup->addButton(but);
        ui->tableEqcLayout->addWidget(but);
    }

    //Adding to list not found items greyed
    if( !found )
    {
        QRadioButton* but = new QRadioButton(ui->tableEqc);
        QString name = QFileInfo(active).baseName();
        but->setText( name );
        but->setDown(true);
        but->setDisabled(true);

//        buttonGroup->addButton(but);
        ui->tableEqcLayout->addWidget(but);
    }
//    ui->tableEqc->setLayout(layout);
}

QString EnvControl::getFile()
{
    return _file;
}

QString EnvControl::getCommand()
{
    return ui->commandLine->text();
}
//
//void EnvControl::setCommand(QString const &com)
//{
//    ui->commandLine->setText(com);
//}

void EnvControl::writeSettings(QSettings* settings)
{
    settings->setValue("file", _file);
    settings->setValue("command", ui->commandLine->text());
    settings->setValue("expanded", _treeItem->isExpanded());
    settings->setValue("hidden", this->isHidden());
    settings->setValue("currPage", ui->stack->currentIndex());
    settings->beginGroup("treeItem");
    settings->setValue("env", _treeItem->isSelected());
    settings->setValue("plugins", _treeItem->plugins->isSelected());
    settings->setValue("eqc", _treeItem->eqc->isSelected());
    settings->setValue("tracking", _treeItem->tracking->isSelected());
    settings->endGroup();

}

void EnvControl::readSettings(QSettings* settings)
{
    ui->commandLine->setText(settings->value("command", "").toString());
    _treeItem->setExpanded(settings->value("Expanded", false).toBool());
    if(settings->value("hidden", true).toBool())
    {
        this->hide();
    }
    ui->stack->setCurrentIndex(settings->value("currPage", 0).toInt());
    settings->beginGroup("treeItem");
    _treeItem->setSelected(settings->value("env", false).toBool());
    _treeItem->plugins->setSelected(settings->value("plugins", false).toBool());
    _treeItem->eqc->setSelected(settings->value("eqc", false).toBool());
    _treeItem->tracking->setSelected(settings->value("tracking", false).toBool());
    settings->endGroup();
}
