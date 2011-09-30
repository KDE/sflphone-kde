
#include "ContactDock.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QListWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QHeaderView>
#include <QtGui/QCheckBox>
#include <QtCore/QDateTime>
#include <QSplitter>
#include <klineedit.h>
#include <KLocalizedString>
#include <QtGui/QLabel>

#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/contact/contactstreemodel.h>
#include <akonadi/kmime/messagemodel.h>
#include <akonadi/changerecorder.h>
#include <kabc/addressee.h>
#include <kabc/picture.h>
#include <kabc/phonenumber.h>
#include <kabc/vcard.h>
#include <kabc/field.h>
#include <kabc/vcardline.h>
#include <kabc/contactgroup.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/recursiveitemfetchjob.h>
#include <kicon.h>
#include <akonadi/entitytreeview.h>
#include <akonadi/itemview.h>
#include <akonadi/collectioncombobox.h>

#include "AkonadiBackend.h"
#include "ContactItemWidget.h"
#include "conf/ConfigurationSkeleton.h"
#include "lib/Call.h"
#include "SFLPhone.h"
#include "lib/Contact.h"

///Hack around Qt sorting limitation
class QNumericTreeWidgetItem_hist : public QTreeWidgetItem {
   public:
      QNumericTreeWidgetItem_hist(QTreeWidget* parent):QTreeWidgetItem(parent),widget(0),weight(-1){}
      QNumericTreeWidgetItem_hist(QTreeWidgetItem* parent):QTreeWidgetItem(parent),widget(0),weight(-1){}
      ContactItemWidget* widget;
      QString number;
      int weight;
   private:
      bool operator<(const QTreeWidgetItem & other) const {
         int column = treeWidget()->sortColumn();
         //if (dynamic_cast<QNumericTreeWidgetItem_hist*>((QTreeWidgetItem*)&other)) {
            //if (widget !=0 && dynamic_cast<QNumericTreeWidgetItem_hist*>((QTreeWidgetItem*)&other)->widget != 0)
            //   return widget->getTimeStamp() < dynamic_cast<QNumericTreeWidgetItem_hist*>((QTreeWidgetItem*)&other)->widget->getTimeStamp();
            //else if (weight > 0 && dynamic_cast<QNumericTreeWidgetItem_hist*>((QTreeWidgetItem*)&other)->weight > 0)
            //   return weight > dynamic_cast<QNumericTreeWidgetItem_hist*>((QTreeWidgetItem*)&other)->weight;
         //}
         return text(column) < other.text(column);
      }
};

///Forward keypresses to the filter line edit
bool KeyPressEaterC::eventFilter(QObject *obj, QEvent *event)
{
   if (event->type() == QEvent::KeyPress) {
      m_pDock->keyPressEvent((QKeyEvent*)event);
      return true;
   } else {
      // standard event processing
      return QObject::eventFilter(obj, event);
   }
}

///Constructor
ContactDock::ContactDock(QWidget* parent) : QDockWidget(parent)
{
   setObjectName("contactDock");
   m_pFilterLE     = new KLineEdit   (                   );
   m_pSplitter     = new QSplitter   ( Qt::Vertical,this );
   m_pSortByCBB    = new QComboBox   ( this              );
   m_pContactView  = new ContactTree ( this              );
   m_pCallView     = new QListWidget ( this              );
   m_pShowHistoCK  = new QCheckBox   ( this              );


   QStringList sortType;
   sortType << "Name" << "Organisation" << "Phone number type" << "Rencently used" << "Group";
   m_pSortByCBB->addItems(sortType);

   QWidget* mainWidget = new QWidget(this);
   setWidget(mainWidget);

   m_pContactView->headerItem()->setText(0,i18n("Contacts"));
   m_pContactView->header()->setClickable(true);
   m_pContactView->header()->setSortIndicatorShown(true);
   m_pContactView->setAcceptDrops(true);
   m_pContactView->setDragEnabled(true);
   KeyPressEaterC *keyPressEater = new KeyPressEaterC(this);
   m_pContactView->installEventFilter(keyPressEater);

   m_pContactView->setAlternatingRowColors(true);

   m_pFilterLE->setPlaceholderText(i18n("Filter"));
   m_pFilterLE->setClearButtonShown(true);

   m_pShowHistoCK->setChecked(ConfigurationSkeleton::displayContactCallHistory());
   m_pShowHistoCK->setText(i18n("Display history"));

   setHistoryVisible(ConfigurationSkeleton::displayContactCallHistory());

   QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);

   mainLayout->addWidget  ( m_pSortByCBB   );
   mainLayout->addWidget  ( m_pShowHistoCK );
   mainLayout->addWidget  ( m_pSplitter    );
   m_pSplitter->addWidget ( m_pContactView );
   m_pSplitter->addWidget ( m_pCallView    );
   mainLayout->addWidget  ( m_pFilterLE    );

   m_pSplitter->setChildrenCollapsible(true);
   m_pSplitter->setStretchFactor(0,7);
   
   connect (AkonadiBackend::getInstance(),SIGNAL(collectionChanged()),                                   this,        SLOT(reloadContact()                      ));
   connect (m_pContactView,               SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),this,        SLOT(loadContactHistory(QTreeWidgetItem*) ));
   connect (m_pFilterLE,                  SIGNAL(textChanged(QString)),                                  this,        SLOT(filter(QString)                      ));
   connect (m_pShowHistoCK,               SIGNAL(toggled(bool)),                                         this,        SLOT(setHistoryVisible(bool)              ));
   setWindowTitle(i18n("Contact"));
}

///Destructor
ContactDock::~ContactDock()
{

}

///Reload the contact
void ContactDock::reloadContact()
{
   ContactList list = AkonadiBackend::getInstance()->update();
   foreach (Contact* cont, list) {
      ContactItemWidget* aContact  = new ContactItemWidget(m_pContactView);
      QNumericTreeWidgetItem_hist* item = new QNumericTreeWidgetItem_hist(m_pContactView);
      item->widget = aContact;
      aContact->setItem(item);
      aContact->setContact(cont);

      PhoneNumbers numbers =  aContact->getContact()->getPhoneNumbers();
      qDebug() << "Phone count" << numbers.count();
      if (numbers.count() > 1) {
         foreach (Contact::PhoneNumber* number, numbers) {
            QNumericTreeWidgetItem_hist* item2 = new QNumericTreeWidgetItem_hist(item);
            QLabel* numberL = new QLabel("<b>"+number->getType()+":</b>"+number->getNumber(),this);
            item2->number = number->getNumber();
            m_pContactView->setItemWidget(item2,0,numberL);
         }
      }
      else if (numbers.count() == 1) {
         item->number = numbers[0]->getNumber();
      }

      m_pContactView->addTopLevelItem(item);
      m_pContactView->setItemWidget(item,0,aContact);
      m_pContacts << aContact;
   }
}

///Query the call history for all items related to this contact
void ContactDock::loadContactHistory(QTreeWidgetItem* item)
{
   if (m_pShowHistoCK->isChecked()) {
      m_pCallView->clear();
      if (dynamic_cast<QNumericTreeWidgetItem_hist*>(item) != NULL) {
         QNumericTreeWidgetItem_hist* realItem = dynamic_cast<QNumericTreeWidgetItem_hist*>(item);
         foreach (Call* call, SFLPhone::app()->model()->getHistory()) {
            if (realItem->widget != 0) {
               foreach (Contact::PhoneNumber* number, realItem->widget->getContact()->getPhoneNumbers()) {
                  if (number->getNumber() == call->getPeerPhoneNumber()) {
                     m_pCallView->addItem(QDateTime::fromTime_t(call->getStartTimeStamp().toUInt()).toString());
                  }
               }
            }
         }
      }
   }
}

///Filter contact
void ContactDock::filter(QString text)
{
   foreach(ContactItemWidget* item, m_pContacts) {
      bool foundNumber = false;
      foreach (Contact::PhoneNumber* number, item->getContact()->getPhoneNumbers()) {
         foundNumber |= number->getNumber().toLower().indexOf(text) != -1;
      }
      bool visible = (item->getContact()->getFormattedName().toLower().indexOf(text) != -1)
                  || (item->getContact()->getOrganization().toLower().indexOf(text) != -1)
                  || (item->getContact()->getPreferredEmail().toLower().indexOf(text) != -1)
                  || foundNumber;
      item->getItem()->setHidden(!visible);
   }
   m_pContactView->expandAll();
}

///Serialize informations to be used for drag and drop
QMimeData* ContactTree::mimeData( const QList<QTreeWidgetItem *> items) const
{
   qDebug() << "An history call is being dragged";
   if (items.size() < 1) {
      return NULL;
   }

   QMimeData *mimeData = new QMimeData();

   //Contact
   if (dynamic_cast<QNumericTreeWidgetItem_hist*>(items[0])) {
      QNumericTreeWidgetItem_hist* item = dynamic_cast<QNumericTreeWidgetItem_hist*>(items[0]);
      if (item->widget != 0) {
         mimeData->setData(MIME_CONTACT, item->widget->getContact()->getUid().toUtf8());
      }
      else if (!item->number.isEmpty()) {
         mimeData->setData(MIME_PHONENUMBER, item->number.toUtf8());
      }
   }
   else {
      qDebug() << "the item is not a call";
   }
   return mimeData;
}

///Handle data being dropped on the widget
bool ContactTree::dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action)
{
   Q_UNUSED(index)
   Q_UNUSED(action)
   Q_UNUSED(parent)

   QByteArray encodedData = data->data(MIME_CALLID);

   qDebug() << "In history import"<< QString(encodedData);

   return false;
}

///Show or hide the history list 
void ContactDock::setHistoryVisible(bool visible)
{
   qDebug() << "Toggling history visibility";
   m_pCallView->setVisible(visible);
   ConfigurationSkeleton::setDisplayContactCallHistory(visible);
}

///Handle keypresses ont the dock
void ContactDock::keyPressEvent(QKeyEvent* event) {
   int key = event->key();
   if(key == Qt::Key_Escape)
      m_pFilterLE->setText(QString());
   else if(key == Qt::Key_Return || key == Qt::Key_Enter) {}
   else if((key == Qt::Key_Backspace) && (m_pFilterLE->text().size()))
      m_pFilterLE->setText(m_pFilterLE->text().left( m_pFilterLE->text().size()-1 ));
   else if (!event->text().isEmpty() && !(key == Qt::Key_Backspace))
      m_pFilterLE->setText(m_pFilterLE->text()+event->text());
}