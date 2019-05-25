#include "core.h"


void Object::deleteLink(Link* link)
{
    for(int i = 0; i < links.size();i++)
        if(links[i] == link)
            links.erase(links.begin() + i);
}

void Graph::push_back(Object* object)
{
    objects.push_back(object);
}

Object* Graph::findObject(QString name)
{
    for(auto e: objects)
        if(e->name == name)
            return e;
    return Q_NULLPTR;
}

std::vector<std::vector<QString>>
    Graph::findPaths(Object* begin, Object* end, bool flag)
{
    if(begin != Q_NULLPTR && end != Q_NULLPTR){
        if(begin->name != end->name){
            std::vector<std::vector<QString>> pathes;
            std::vector<QString> path;
            path.push_back(begin->name);
            DFS(begin,end,pathes,path,flag);
            return pathes;
        }
    }

}

void Graph::DFS(Object* cur, Object* target,
         std::vector<std::vector<QString>> &pathes,
         std::vector<QString> &path,bool flag)
{

    for(auto e:cur->links){

        Object* curObject = Q_NULLPTR;
        if(flag){
            if(e->receiver->name != cur->name){
                path.push_back(e->name);
                path.push_back(e->receiver->name);
                curObject = e->receiver;
            }
            else break;
        }
        else{
            if(e->receiver->name == cur->name){
                path.push_back(e->name);
                path.push_back(e->source->name);
                curObject = e->source;
            }
            else {
                path.push_back(e->name);
                path.push_back(e->receiver->name);
                curObject = e->receiver;
            }
        }

        if(curObject != target)
            DFS(curObject,target,pathes,path,flag);
        else {
            pathes.push_back(path);
            path.pop_back();
            path.pop_back();
            if(path.size()>2){
                path.pop_back();
                path.pop_back();
            }
            return;
        }
    }
    if(path.size()>2){
        path.pop_back();
        path.pop_back();
    }
    return;
}

void Graph::erase(Object* object)
{
    for(int i = 0; i < objects.size();i++)
        if(objects[i] == object)
            objects.erase(objects.begin()+i);
}

QString toString(LinkTypes linkType)
{
    if(linkType == Gen) return "Gen";
    if(linkType == Sit) return "Sit";
    if(linkType == Neg) return "Neg";
    if(linkType == Ins) return "Ins";
    if(linkType == Com) return "Com";
    if(linkType == Cor) return "Cor";
    if(linkType == Fin) return "Fin";
    if(linkType == Caus) return "Caus";
    if(linkType == Pot) return "Pot";
}
