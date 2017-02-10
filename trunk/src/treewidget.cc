/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#include "treewidget.h"
#include <QStandardItem>
#include "debug.h"

class SortDisabler {
public:
  SortDisabler(QTreeWidget* tree) {
    m_qtwTree = tree;
    if (m_qtwTree)
      m_qtwTree->setSortingEnabled(false);
  }

  ~SortDisabler() {
    if (m_qtwTree)
      m_qtwTree->setSortingEnabled(true);
  }

protected:
  QTreeWidget* m_qtwTree;
};

cobraTreeWidget::cobraTreeWidget(QWidget *parent) :
  QTreeWidget(parent), cobraClipList()
{

  connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(clipItemChanged(QTreeWidgetItem*, int)));
}

void
cobraTreeWidget::showUpload(bool t)
{
  setColumnHidden(Upload, !t);
}

void
cobraTreeWidget::showDownload(bool t)
{
  setColumnHidden(Download, !t);
}

void
cobraTreeWidget::showUid(bool t)
{
  setColumnHidden(ID, !t);
}

void
cobraTreeWidget::configure()
{
  /* Align the colums to the left */
  setIndentation(0);

  setColumnWidth(Upload, 30);
  setColumnWidth(Download, 30);
  setColumnWidth(ID, 10);
  setColumnWidth(Hash, 48);
  setColumnWidth(Title, 120);
  setColumnWidth(Description, 205);
  setColumnWidth(ModifiedTime, 150);
  setColumnWidth(Tags, 90);
}

bool
cobraTreeWidget::setName(QString db)
{
  this->clear();
  if (!cobraClipList::setName(db))
    return false;

  repopulateTree();
  return true;
}

void
cobraTreeWidget::repopulateTree()
{
  SortDisabler sort(this);
  QTreeWidgetItem *itm = NULL;
  QVector<int> uids;
  cobraClip clip;

  enumClips(uids);

  for (int x=0; x<uids.size(); x++) {
      clip = getClip(uids[x]);
      debug(HIGH, "Populating local list: %d\n", uids[x]);
      itm = new QTreeWidgetItem(this);

      itm->setCheckState(Upload, Qt::Unchecked);
      itm->setCheckState(Download, Qt::Unchecked);
      itm->setText(ID, QString::number(clip.getUid()));
      itm->setText(Hash, clip.getHash().left(4));
      itm->setText(Title, clip.getTitle());
      itm->setText(ModifiedTime, clip.getModifiedTime());
      itm->setText(Description, clip.getDescription());

      itm->setText(Tags, clip.getTags());

      this->addTopLevelItem(itm);
    }
}

void
cobraTreeWidget::setFilter(QString filter)
{
  SortDisabler sort(this);
  QList<QTreeWidgetItem*> items;

  if (filter.isEmpty()) {
      items = findItems("*", Qt::MatchWildcard, cobraTreeWidget::ID);
      for (int i=0; i<items.size(); i++)
          items[i]->setHidden(false);
      return;
    }

  items = findItems("*", Qt::MatchWildcard, cobraTreeWidget::ID);
  for (int i=0; i<items.size(); i++)
      items[i]->setHidden(true);

  extern QSettings* g_cobra_settings;
  QString mode = g_cobra_settings->value("filter").toString();
  int filterMode = Qt::MatchRegExp;
  if (mode == "Wild Card")
    filterMode = Qt::MatchWildcard;
  else if (mode == "Contains")
    filterMode = Qt::MatchContains;

  if (filterMode == Qt::MatchRegExp) {
    if (filter.begin()->isLetter())
      filter.prepend(".*");
    if ((filter.end()-1)->isLetter())
      filter.append(".*");
  }

  for (int i=cobraTreeWidget::Hash; i<=cobraTreeWidget::ModifiedTime; i++) {
      items = findItems(filter, (Qt::MatchFlags)filterMode, i);
      for (int x=0; x<items.size(); x++)
          items[x]->setHidden(false);
  }

}

bool
cobraTreeWidget::updateClip(cobraClip& clip)
{
  bool ret = cobraClipList::updateClip(clip);
  if (!ret) {
      debug(CRITICAL, "Ah, shiznit, clipList rejected out shit!\n");
      return ret;
    }

  SortDisabler sort(this);

  /**
     * This just updates the clip in teh database --
     *
     * This should NOT be the function called when a user selected to edit a Clip's information,
     * this SHOULD be the function called AFTER the user has selected to SAVE the clip information.
     */

  QString clipid = QString::number(clip.getUid());
  QString cliphash = clip.getHash().left(4);
  QString cliptitle = clip.getTitle();
  QString clipdesc = clip.getDescription();
  QString cliptime = clip.getModifiedTime();
  QString cliptags = clip.getTags();

  QList<QTreeWidgetItem*> c = this->findItems(clipid, Qt::MatchExactly, cobraTreeWidget::ID);

  if(c.size() != 1) {
      debug(ERROR(HIGH), "Failed to find specified ID!\n");
      return false;
    }

  QTreeWidgetItem* itm = c.takeFirst();

  itm->setCheckState(Upload, Qt::Unchecked);
  itm->setCheckState(Download, Qt::Unchecked);
  itm->setText(ID, clipid);
  itm->setText(Hash, cliphash);
  itm->setText(Title, cliptitle);
  itm->setText(Description, clipdesc);
  itm->setText(ModifiedTime, cliptime);
  itm->setText(Tags, cliptags);

  m_bChecked = true;

  return true;
}

bool
cobraTreeWidget::removeClip(int uid)
{
  bool ret = cobraClipList::removeClip(uid);
  if (!ret)
    return ret;

  SortDisabler sort(this);
  QList<QTreeWidgetItem*> c = this->findItems(QString::number(uid), Qt::MatchExactly, cobraTreeWidget::ID);

  if(c.size() != 1) {
      debug(ERROR(HIGH), "Failed to find specified ID!\n");
      return false;
    }

  delete c.takeFirst();

  return true;
}

bool
cobraTreeWidget::addClip(cobraClip& clip)
{
  bool ret = cobraClipList::addClip(clip);
  if (!ret)
    return ret;

  SortDisabler sort(this);
  QTreeWidgetItem *itm = new QTreeWidgetItem(this);

  QString cliptitle = clip.getTitle();
  QString clipid = QString::number(clip.getUid());
  QString cliphash = clip.getHash().left(4);
  QString clipdesc = clip.getDescription();
  QString cliptime = clip.getModifiedTime();
  QString cliptags = clip.getTags();

  itm->setCheckState(Upload, Qt::Unchecked);
  itm->setCheckState(Download, Qt::Unchecked);
  itm->setText(ID, clipid);
  itm->setText(Hash, cliphash);
  itm->setText(Title, cliptitle);
  itm->setText(Description, clipdesc);
  itm->setText(ModifiedTime, cliptime);
  itm->setText(Tags, cliptags);
  this->addTopLevelItem(itm);

  m_bChecked = true;

  return true;
}

void
cobraTreeWidget::getSelectedUids(QVector<int>& uids)
{
  QList<QTreeWidgetItem *> items = this->selectedItems();
  for(int i = 0; i < items.size(); i++) {
      QTreeWidgetItem *itm = items.at(i);

      if (!itm)
        continue;

      uids.append(itm->text(ID).toInt(0,10));
    }
}

void
cobraTreeWidget::getCheckedUids(QVector<int>& uids)
{
  QList<QTreeWidgetItem *> items = this->findItems("*", Qt::MatchWildcard);
  debug(LOW, "Checked Items List Size: %d\n", items.size());

  for(int i = 0; i < items.size(); i++) {
      QTreeWidgetItem *itm = items.at(i);

      if (!itm)
        continue;

      if (itm->checkState(Upload) != Qt::Checked)
        continue;

      uids.append(itm->text(ID).toInt(0,10));
    }
}

void
cobraTreeWidget::getCheckedUidsDownload(QVector<int>& uids)
{
  QList<QTreeWidgetItem *> items = this->findItems("*", Qt::MatchWildcard);
  debug(LOW, "Checked Items List Size: %d\n", items.size());

  for(int i = 0; i < items.size(); i++) {
      QTreeWidgetItem *itm = items.at(i);

      if (!itm)
        continue;

      if (itm->checkState(Download) != Qt::Checked)
        continue;

      uids.append(itm->text(ID).toInt(0,10));
    }
}

bool
cobraTreeWidget::syncable() const
{
  return m_bChecked;
}

void
cobraTreeWidget::clipItemChanged(QTreeWidgetItem* item, int index)
{
  bool checked = false;
  (void)item; (void)index;
  QList<QTreeWidgetItem *> items = this->findItems("*", Qt::MatchWildcard, ID);

  debug(CRITICAL, "Clip item changes\n");

  for(int i = 0; i < items.size(); i++) {
      QTreeWidgetItem *itm = items.at(i);

      if (!itm)
        continue;

      if (itm->checkState(Download) != Qt::Checked)
        continue;

      checked = true;
      break;
    }

  debug(CRITICAL, "Download: %d\n", checked);

  emit downloadSelected(checked);
}

void
cobraTreeWidget::synchronized()
{
  QList<QTreeWidgetItem *> items = this->findItems("*", Qt::MatchWildcard);

  m_bChecked = false;

  for(int i = 0; i < items.size(); i++) {
      QTreeWidgetItem *itm = items.at(i);

      if (!itm)
        continue;

      itm->setCheckState(Upload, Qt::Unchecked);
    }
}

void
cobraTreeWidget::setUpload(int uid, bool t)
{
  QList<QTreeWidgetItem*> c = this->findItems(QString::number(uid), Qt::MatchExactly, cobraTreeWidget::ID);
  if (!c.size())
    return;

  c.at(0)->setCheckState(Upload, t?Qt::Checked:Qt::Unchecked);
}

void
cobraTreeWidget::setDownload(int uid, bool t)
{
  QList<QTreeWidgetItem*> c = this->findItems(QString::number(uid), Qt::MatchExactly, cobraTreeWidget::ID);
  if (!c.size())
    return;

  c.at(0)->setCheckState(Download, t?Qt::Checked:Qt::Unchecked);
}

void
cobraTreeWidget::uncheckAll(int id)
{
  if (id != Download && id != Upload)
    return;

  QList<QTreeWidgetItem *> items = this->findItems("*", Qt::MatchWildcard);
  for (int x=0; x<items.size(); x++) {
      items[x]->setCheckState(id, Qt::Unchecked);
    }
}
