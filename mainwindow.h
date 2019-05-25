#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include "core.h"
#include "gui.h"
#include "dialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent *event);
public slots:
    void paintNewObject();
    void paintNewLink(int linkType);
    void changeEditedWidget(bool);
    void createDialogWindow();
    void findPath(QString begin,QString end,bool flag);
    void saveFileAs();
    void openFile();
    void exit();
    void closeFile();
    void saveFile();
    void deleteObjectWidget();
    void deleteLinkWidget();
signals:
    void changingWidget(bool);
private:
    void createContextMenu();
    void createMainMenu();
    void reconnect();
    void saveSemanticNetwork(QString filename);
    void loadSemanticNetwork(QString filename);
    void clear();
    Ui::MainWindow *ui;
    QWidget* editedWidget = Q_NULLPTR;
    QMenu* contextMenu = Q_NULLPTR;
    QMenu* linkMenu = Q_NULLPTR;
    bool createObjectFlag = false;
    bool createLinkFlag = false;
    LinkTypes createdLinkType;
    std::vector<ObjectWidget*> objectWidgets;
    std::vector<LinkWidget*> linkWidgets;
    Graph* graph;
    QString filename;
};

#endif // MAINWINDOW_H
