#ifndef CORE_H
#define CORE_H




#include <QString>
#include <vector>
#include <QPainter>
#include <QPoint>
#include <QLineEdit>
#include <QLabel>

enum LinkTypes{
    Gen, // генеративная
    Sit, // ситуационная
    Neg, // негативная
    Ins, // инструментальная
    Com, // комитативная
    Cor, // корреляционная
    Fin, // финитивная
    Caus, // казуальная
    Pot // потенсивная
};

QString toString(LinkTypes linkType);

class Object;

class Link
{
public:
    Link(){}
    QString name;
    LinkTypes linkType;
    Object *receiver = Q_NULLPTR;
    Object *source = Q_NULLPTR;
};


class Object
{
public:
    Object(){}
    void deleteLink(Link* link);
    void addLink(Link* link){links.push_back(link);}
    QString name;
    std::vector<Link*> links;
};

class Graph
{
public:
    void push_back(Object* object);
    Object* findObject(QString name);
    std::vector<std::vector<QString>>
        findPaths(Object* begin, Object* end, bool flag);
    void erase(Object* object);
private:
    void DFS(Object* cur, Object* target,
             std::vector<std::vector<QString>> &pathes,
             std::vector<QString> &path,bool flag);
    std::vector<Object*> objects;
};
#endif // CORE_H


