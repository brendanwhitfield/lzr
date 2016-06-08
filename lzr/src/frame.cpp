
#include "frame.h"


Frame::Frame(lzr::Frame& frame)
{
    //split the frame into paths
    bool was_lit = false;
    lzr::Frame path;

    for(lzr::Point& p : frame)
    {
        if(p.is_lit()) //if the laser just turned on
        {
            path.add(p);
        }
        else if(p.is_blanked() && was_lit) //if the laser just turned off
        {
            paths.append(path);
            path.clear();
        }

        was_lit = p.is_lit();
    }

    //if a path was left open, close it
    if(was_lit)
    {
        paths.append(path);
        path.clear();
    }
}

int Frame::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return paths.size();
}

QVariant Frame::data(const QModelIndex& index, int role) const
{
    Q_UNUSED(role);

    if(!index.isValid())
        return QVariant();

    if(index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    QVariant v;
    v.setValue(paths[index.row()]);
    return v;
}

int Frame::columnCount(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return 1;
}

QModelIndex Frame::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    if(column != 0)
        return QModelIndex();

    if(row >= rowCount())
        return QModelIndex();

    return createIndex(row, 0);
}

QModelIndex Frame::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}
