#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "base/projsettings.hpp"
#include "base/envsettings.hpp"

#include "envcontrol.h"
#include "projectcontrol.h"

#include <boost/shared_ptr.hpp>

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>
#include <QListWidget>
#include <QDir>
#include <QStringList>
#include <QSettings>


namespace Ui {
    class MainWindow;
}

const QColor RED(255,0,0);
const QString RED_STYLE("color: rgb(150,0,0)");
const QColor WHITE(255,255,255);
const QString WHITE_STYLE("color: rgb(0,0,0)");

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots :
    void launched(QString const &file, QString const &name);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    //std::vector<std::string> _currentPlugins;
    //QString _parseDir(QString dir);

//    EnvSettingsRefPtr _envSet;
//    QList<ProjSettingsRefPtr> _projSets;

    EnvControl* _envCtrl;
    QList<ProjControl*> _projCtrls;

    QTreeWidgetItem* _currFocusTreeItem;

    void readSettings();
    void writeSettings();
    void createProjCtrl(QString const &file);

    bool isProjectTreeItem(QTreeWidgetItem* item);

private slots:
    void on_actionSave_triggered();
    void on_tree_customContextMenuRequested(QPoint pos);
    void on_actionOpenProject_triggered();
    void on_actionOpenEnvironment_triggered();
    void on_actionExit_triggered();
    void on_actionClose_Project_triggered();

//    void on_selectedDotscenesTable_itemChanged(QTableWidgetItem* item);
//    void on_sceneAddButton_clicked();


   // void initProjectTree( ProjSettingsRefPtr set );
};

#endif // MAINWINDOW_H
