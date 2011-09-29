/***************************************************************************
 *   Copyright (C) 2009-2010 by Savoir-Faire Linux                         *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>         *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>*
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 **************************************************************************/

#include "SFLPhoneView.h"

#include <QtGui/QLabel>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QBrush>
#include <QtGui/QPalette>
#include <QtGui/QInputDialog>

#include <klocale.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kmenu.h>

#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addresseelist.h>

#include "lib/sflphone_const.h"
#include "conf/ConfigurationSkeleton.h"
#include "lib/configurationmanager_interface_singleton.h"
#include "lib/callmanager_interface_singleton.h"
#include "lib/instance_interface_singleton.h"
#include "ActionSetAccountFirst.h"
#include "widgets/ContactItemWidget.h"
#include "SFLPhone.h"
#include "lib/typedefs.h"
#include "widgets/Dialpad.h"
#include "widgets/CallTreeItem.h"


using namespace KABC;

//ConfigurationDialog* SFLPhoneView::configDialog;

SFLPhoneView::SFLPhoneView(QWidget *parent)
   : QWidget(parent),
     wizard(0)
{
   setupUi(this);
   
   ConfigurationManagerInterface& configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
   
   errorWindow = new QErrorMessage(this);
   callTreeModel->setTitle("Calls");
   
   QPalette pal = QPalette(palette());
   pal.setColor(QPalette::AlternateBase, Qt::lightGray);
   setPalette(pal);
   
   connect(SFLPhone::model()                     , SIGNAL(incomingCall(Call*))                   , this                                  , SLOT(on1_incomingCall(Call*)                    ));
   connect(SFLPhone::model()                     , SIGNAL(voiceMailNotify(const QString &, int)) , this                                  , SLOT(on1_voiceMailNotify(const QString &, int)  ));
   connect(SFLPhone::model()                     , SIGNAL(volumeChanged(const QString &, double)), this                                  , SLOT(on1_volumeChanged(const QString &, double) ));
   connect(SFLPhone::model()                     , SIGNAL(callStateChanged(Call*))               , this                                  , SLOT(updateWindowCallState()                    ));
   connect(TreeWidgetCallModel::getAccountList() , SIGNAL(accountListUpdated())                  , this                                  , SLOT(updateStatusMessage()                      ));
   connect(TreeWidgetCallModel::getAccountList() , SIGNAL(accountListUpdated())                  , this                                  , SLOT(updateWindowCallState()                    ));
   connect(&configurationManager                 , SIGNAL(accountsChanged())                     , TreeWidgetCallModel::getAccountList() , SLOT(updateAccounts()                           ));

   TreeWidgetCallModel::getAccountList()->updateAccounts();
}



SFLPhoneView::~SFLPhoneView()
{
}

void SFLPhoneView::saveState()
{
   //ConfigurationManagerInterface & configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
   //configurationManager.setHistory(callTreeModel->getHistoryCallId());
}

void SFLPhoneView::loadWindow()
{
   updateWindowCallState ();
   updateRecordButton    ();
   updateVolumeButton    ();
   updateRecordBar       ();
   updateVolumeBar       ();
   updateVolumeControls  ();
   updateDialpad         ();
   updateStatusMessage   ();
}

QErrorMessage * SFLPhoneView::getErrorWindow()
{
   return errorWindow;
}

// CallView* SFLPhoneView::model()
// {
//    return callTreeModel;
// }

void SFLPhoneView::typeString(QString str)
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   
   Call* call = callTreeModel->getCurrentItem();
   callManager.playDTMF(str);
   Call *currentCall = 0;
   Call *candidate = 0;

   if(call) {
      if(call->getState() == CALL_STATE_CURRENT) {
         currentCall = call;
      }
   }

   foreach (Call* call2, SFLPhone::model()->getCallList()) {
      if(currentCall != call2 && call2->getState() == CALL_STATE_CURRENT) {
         action(call2, CALL_ACTION_HOLD);
      }
      else if(call2->getState() == CALL_STATE_DIALING) {
         candidate = call2;
      }
   }

   if(!currentCall && !candidate) {
      qDebug() << "Typing when no item is selected. Opening an item.";
      candidate = SFLPhone::model()->addDialingCall();
   }

   if(!currentCall && candidate) {
      candidate->appendText(str);
   }
}

void SFLPhoneView::backspace()
{
   qDebug() << "backspace";
   qDebug() << "In call list.";
   Call* call = callTreeModel->getCurrentItem();
   if(!call) {
      qDebug() << "Error : Backspace on unexisting call.";
   }
   else {
      call->backspaceItemText();
      if(call->getState() == CALL_STATE_OVER) {
         if (callTreeModel->getCurrentItem())
            callTreeModel->removeItem(callTreeModel->getCurrentItem());
      }
   }
}

void SFLPhoneView::escape()
{
   qDebug() << "escape";
   Call* call = callTreeModel->getCurrentItem();
   if(!call) {
      qDebug() << "Escape when no item is selected. Doing nothing.";
   }
   else {
      if(call->getState() == CALL_STATE_TRANSFER || call->getState() == CALL_STATE_TRANSF_HOLD) {
         action(call, CALL_ACTION_TRANSFER);
      }
      else {
         action(call, CALL_ACTION_REFUSE);
      }
   }
}

void SFLPhoneView::enter()
{
   qDebug() << "enter";
   Call* call = callTreeModel->getCurrentItem();
   if(!call) {
      qDebug() << "Error : Enter on unexisting call.";
   }
   else {
      int state = call->getState();
      if(state == CALL_STATE_INCOMING || state == CALL_STATE_DIALING || state == CALL_STATE_TRANSFER || state == CALL_STATE_TRANSF_HOLD) {
         action(call, CALL_ACTION_ACCEPT);
      }
      else {
         qDebug() << "Enter when call selected not in appropriate state. Doing nothing.";
      }
   }
}

void SFLPhoneView::action(Call* call, call_action action)
{
   if(! call) {
      qDebug() << "Error : action " << action << "applied on null object call. Should not happen.";
   }
   else {
      try {
         call->actionPerformed(action);
      }
      catch(const char * msg) {
         errorWindow->showMessage(QString(msg));
      }
      updateWindowCallState();
   }
}


/*******************************************
******** Update Display Functions **********
*******************************************/

void SFLPhoneView::updateWindowCallState()
{
   qDebug() << "updateWindowCallState";
   
   bool enabledActions[6]= {true,true,true,true,true,true};
   QString buttonIconFiles[6] = {ICON_CALL, ICON_HANGUP, ICON_HOLD, ICON_TRANSFER, ICON_REC_DEL_OFF, ICON_MAILBOX};
   QString actionTexts[6] = {ACTION_LABEL_CALL, ACTION_LABEL_HANG_UP, ACTION_LABEL_HOLD, ACTION_LABEL_TRANSFER, ACTION_LABEL_RECORD, ACTION_LABEL_MAILBOX};
   
   Call* call = 0;
   
   bool transfer = false;
   bool recordActivated = false;    //tells whether the call is in recording position

   enabledActions[SFLPhone::Mailbox] = SFLPhone::model()->getCurrentAccount() && ! SFLPhone::model()->getCurrentAccount()->getAccountDetail(ACCOUNT_MAILBOX).isEmpty();

   call = callTreeModel->getCurrentItem();
   if (!call) {
      qDebug() << "No item selected.";
      enabledActions[ SFLPhone::Refuse   ] = false;
      enabledActions[ SFLPhone::Hold     ] = false;
      enabledActions[ SFLPhone::Transfer ] = false;
      enabledActions[ SFLPhone::Record   ] = false;
   }
   else {
      call_state state = call->getState();
      recordActivated = call->getRecording();

      qDebug() << "Reached  State" << state << " with call" << call->getCallId();

      switch (state) {
         case CALL_STATE_INCOMING:
            buttonIconFiles [ SFLPhone::Accept   ] = ICON_ACCEPT                 ;
            buttonIconFiles [ SFLPhone::Refuse   ] = ICON_REFUSE                 ;
            actionTexts     [ SFLPhone::Accept   ] = ACTION_LABEL_ACCEPT         ;
            actionTexts     [ SFLPhone::Refuse   ] = ACTION_LABEL_REFUSE         ;
            break;
         case CALL_STATE_RINGING:
            enabledActions  [ SFLPhone::Hold     ] = false                       ;
            enabledActions  [ SFLPhone::Transfer ] = false                       ;
            break;
         case CALL_STATE_CURRENT:
            buttonIconFiles [ SFLPhone::Record   ] = ICON_REC_DEL_ON             ;
            break;
         case CALL_STATE_DIALING:
            enabledActions  [ SFLPhone::Hold     ] = false                       ;
            enabledActions  [ SFLPhone::Transfer ] = false                       ;
            enabledActions  [ SFLPhone::Record   ] = false                       ;
            actionTexts     [ SFLPhone::Accept   ] = ACTION_LABEL_ACCEPT         ;
            buttonIconFiles [ SFLPhone::Accept   ] = ICON_ACCEPT                 ;
            break;
         case CALL_STATE_HOLD:
            buttonIconFiles [ SFLPhone::Hold     ] = ICON_UNHOLD                 ;
            actionTexts     [ SFLPhone::Hold     ] = ACTION_LABEL_UNHOLD         ;
            break;
         case CALL_STATE_FAILURE:
            enabledActions  [ SFLPhone::Accept   ] = false                       ;
            enabledActions  [ SFLPhone::Hold     ] = false                       ;
            enabledActions  [ SFLPhone::Transfer ] = false                       ;
            enabledActions  [ SFLPhone::Record   ] = false                       ;
            break;
         case CALL_STATE_BUSY:
            enabledActions  [ SFLPhone::Accept   ] = false                       ;
            enabledActions  [ SFLPhone::Hold     ] = false                       ;
            enabledActions  [ SFLPhone::Transfer ] = false                       ;
            enabledActions  [ SFLPhone::Record   ] = false                       ;
            break;
         case CALL_STATE_TRANSFER:
            buttonIconFiles [ SFLPhone::Accept   ] = ICON_EXEC_TRANSF            ;
            actionTexts     [ SFLPhone::Transfer ] = ACTION_LABEL_GIVE_UP_TRANSF ;
            buttonIconFiles [ SFLPhone::Record   ] = ICON_REC_DEL_ON             ;
            transfer = true;
            break;
         case CALL_STATE_TRANSF_HOLD:
            buttonIconFiles [ SFLPhone::Accept   ] = ICON_EXEC_TRANSF            ;
            buttonIconFiles [ SFLPhone::Hold     ] = ICON_UNHOLD                 ;
            actionTexts     [ SFLPhone::Transfer ] = ACTION_LABEL_GIVE_UP_TRANSF ;
            actionTexts     [ SFLPhone::Hold     ] = ACTION_LABEL_UNHOLD         ;
            transfer = true;
            break;
         case CALL_STATE_OVER:
            qDebug() << "Error : Reached CALL_STATE_OVER with call "  << call->getCallId() << "!";
            break;
         case CALL_STATE_ERROR:
            qDebug() << "Error : Reached CALL_STATE_ERROR with call " << call->getCallId() << "!";
            break;
         default:
            qDebug() << "Error : Reached unexisting state for call "  << call->getCallId() << "!";
            break;
      }
   }
   
   qDebug() << "Updating Window.";
   
   emit enabledActionsChangeAsked     ( enabledActions  );
   emit actionIconsChangeAsked        ( buttonIconFiles );
   emit actionTextsChangeAsked        ( actionTexts     );
   emit transferCheckStateChangeAsked ( transfer        );
   emit recordCheckStateChangeAsked   ( recordActivated );

   qDebug() << "Window updated.";
}

void SFLPhoneView::alternateColors(QListWidget * listWidget)
{
   for(int i = 0 ; i < listWidget->count(); i++) {
      QListWidgetItem* item = listWidget->item(i);
      QBrush c = (i % 2 == 1) ? palette().base() : palette().alternateBase();
      item->setBackground( c );
   }
   listWidget->setUpdatesEnabled( true );

}

int SFLPhoneView::phoneNumberTypesDisplayed()
{
   ConfigurationManagerInterface & configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
   MapStringInt addressBookSettings = configurationManager.getAddressbookSettings().value();
   int typesDisplayed = 0;
   if(addressBookSettings[ADDRESSBOOK_DISPLAY_BUSINESS]) {
      typesDisplayed = typesDisplayed | PhoneNumber::Work;
   }
   
   if(addressBookSettings[ADDRESSBOOK_DISPLAY_MOBILE]) {
      typesDisplayed = typesDisplayed | PhoneNumber::Cell;
   }
   
   if(addressBookSettings[ADDRESSBOOK_DISPLAY_HOME]) {
      typesDisplayed = typesDisplayed | PhoneNumber::Home;
   }
   
   return typesDisplayed;
}

void SFLPhoneView::updateRecordButton()
{
   qDebug() << "updateRecordButton";
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   double recVol = callManager.getVolume(RECORD_DEVICE);
   if(recVol == 0.00) {
      toolButton_recVol->setIcon(QIcon(ICON_REC_VOL_0));
   }
   else if(recVol < 0.33) {
      toolButton_recVol->setIcon(QIcon(ICON_REC_VOL_1));
   }
   else if(recVol < 0.67) {
      toolButton_recVol->setIcon(QIcon(ICON_REC_VOL_2));
   }
   else {
      toolButton_recVol->setIcon(QIcon(ICON_REC_VOL_3));
   }
   
   if(recVol > 0) {   
      toolButton_recVol->setChecked(false);
   }
}
void SFLPhoneView::updateVolumeButton()
{
   qDebug() << "updateVolumeButton";
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   double sndVol = callManager.getVolume(SOUND_DEVICE);
        
   if(sndVol == 0.00) {
      toolButton_sndVol->setIcon(QIcon(ICON_SND_VOL_0));
   }
   else if(sndVol < 0.33) {
      toolButton_sndVol->setIcon(QIcon(ICON_SND_VOL_1));
   }
   else if(sndVol < 0.67) {
      toolButton_sndVol->setIcon(QIcon(ICON_SND_VOL_2));
   }
   else {
      toolButton_sndVol->setIcon(QIcon(ICON_SND_VOL_3));
   }
   
   if(sndVol > 0) {
      toolButton_sndVol->setChecked(false);
   }
}


void SFLPhoneView::updateRecordBar()
{
   qDebug() << "updateRecordBar";
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   double recVol = callManager.getVolume(RECORD_DEVICE);
   int value = (int)(recVol * 100);
   slider_recVol->setValue(value);
}
void SFLPhoneView::updateVolumeBar()
{
   qDebug() << "updateVolumeBar";
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   double sndVol = callManager.getVolume(SOUND_DEVICE);
   int value = (int)(sndVol * 100);
   slider_sndVol->setValue(value);
}

void SFLPhoneView::updateVolumeControls()
{
   ConfigurationManagerInterface & configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
   int display = false;

   if(QString(configurationManager.getAudioManager()) == "alsa") {
      display = true;

      SFLPhone::app()->action_displayVolumeControls->setEnabled(true);
   }
   else {
      SFLPhone::app()->action_displayVolumeControls->setEnabled(false);
   }
      
   SFLPhone::app()->action_displayVolumeControls->setChecked(display);
   //widget_recVol->setVisible(display);
   //widget_sndVol->setVisible(display);
   toolButton_recVol->setVisible ( display && ConfigurationSkeleton::displayVolume() );
   toolButton_sndVol->setVisible ( display && ConfigurationSkeleton::displayVolume() );
   slider_recVol->setVisible     ( display && ConfigurationSkeleton::displayVolume() );
   slider_sndVol->setVisible     ( display && ConfigurationSkeleton::displayVolume() );
   
}

void SFLPhoneView::updateDialpad()
{
   widget_dialpad->setVisible(ConfigurationSkeleton::displayDialpad());//TODO use display variable
}


void SFLPhoneView::updateStatusMessage()
{
   qDebug() << "updateStatusMessage";
   Account * account = SFLPhone::model()->getCurrentAccount();

   if(account == NULL) {
      emit statusMessageChangeAsked(i18n("No registered accounts"));
   }
   else {
      emit statusMessageChangeAsked(i18n("Using account") 
                     + " \'" + account->getAlias() 
                     + "\' (" + account->getAccountDetail(ACCOUNT_TYPE) + ")") ;
   }
}



/************************************************************
************            Autoconnect             *************
************************************************************/

void SFLPhoneView::displayVolumeControls(bool checked)
{
   //ConfigurationManagerInterface & configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
   ConfigurationSkeleton::setDisplayVolume(checked);
   updateVolumeControls();
}

void SFLPhoneView::displayDialpad(bool checked)
{
   qDebug() <<  "Max res2: " << ConfigurationSkeleton::displayDialpad();
   ConfigurationSkeleton::setDisplayDialpad(checked);
   updateDialpad();
}


void SFLPhoneView::on_widget_dialpad_typed(QString text)      
{ 
   typeString(text); 
}

void SFLPhoneView::on_slider_recVol_valueChanged(int value)
{
   qDebug() << "on_slider_recVol_valueChanged(" << value << ")";
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   callManager.setVolume(RECORD_DEVICE, (double)value / 100.0);
   updateRecordButton();
}

void SFLPhoneView::on_slider_sndVol_valueChanged(int value)
{
   qDebug() << "on_slider_sndVol_valueChanged(" << value << ")";
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   callManager.setVolume(SOUND_DEVICE, (double)value / 100.0);
   updateVolumeButton();
}

void SFLPhoneView::on_toolButton_recVol_clicked(bool checked)
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   qDebug() << "on_toolButton_recVol_clicked().";
   if(!checked) {
      qDebug() << "checked";
      toolButton_recVol->setChecked(false);
      slider_recVol->setEnabled(true);
      callManager.setVolume(RECORD_DEVICE, (double)slider_recVol->value() / 100.0);
   }
   else {
      qDebug() << "unchecked";
      toolButton_recVol->setChecked(true);
      slider_recVol->setEnabled(false);
      callManager.setVolume(RECORD_DEVICE, 0.0);
   }
   updateRecordButton();
}

void SFLPhoneView::on_toolButton_sndVol_clicked(bool checked)
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   qDebug() << "on_toolButton_sndVol_clicked().";
   if(!checked) {
      qDebug() << "checked";
      toolButton_sndVol->setChecked(false);
      slider_sndVol->setEnabled(true);
      callManager.setVolume(SOUND_DEVICE, (double)slider_sndVol->value() / 100.0);
   }
   else {
      qDebug() << "unchecked";
      toolButton_sndVol->setChecked(true);
      slider_sndVol->setEnabled(false);
      callManager.setVolume(SOUND_DEVICE, 0.0);
   }
   
   updateVolumeButton();
}

// void SFLPhoneView::on_callTree_currentItemChanged()
// {
//    qDebug() << "on_callTree_currentItemChanged";
//    updateWindowCallState();
// }

// void SFLPhoneView::on_callTree_itemChanged()
// {
//    qDebug() << "on_callTree_itemChanged";
// }

//void SFLPhoneView::on_callTree_itemDoubleClicked(QTreeWidgetItem* call, int column)
//{
//Q_UNUSED(call)
//Q_UNUSED(column)
   //TODO port
   //TODO remove once the last regression is sorted out.
//    qDebug() << "on_callTree_itemDoubleClicked";
//    call_state state = call->getCurrentState();
//    switch(state) {
//       case CALL_STATE_HOLD:
//          action(call, CALL_ACTION_HOLD);
//          break;
//       case CALL_STATE_DIALING:
//          action(call, CALL_ACTION_ACCEPT);
//          break;
//       default:
//          qDebug() << "Double clicked an item with no action on double click.";
//    }
//}

void SFLPhoneView::contextMenuEvent(QContextMenuEvent *event)
{
   KMenu menu(this);
   
   SFLPhone * window = SFLPhone::app();
   QList<QAction *> callActions = window->getCallActions();
   
   menu.addAction ( callActions.at((int) SFLPhone::Accept) );
   menu.addAction ( callActions[ SFLPhone::Refuse   ]      );
   menu.addAction ( callActions[ SFLPhone::Hold     ]      );
   menu.addAction ( callActions[ SFLPhone::Transfer ]      );
   menu.addAction ( callActions[ SFLPhone::Record   ]      );
   menu.addSeparator();
   
   QAction* action = new ActionSetAccountFirst(NULL, &menu);
   action->setChecked(SFLPhone::model()->getPriorAccoundId().isEmpty());
   connect(action,  SIGNAL(setFirst(Account *)), this  ,  SLOT(setAccountFirst(Account *)));
   menu.addAction(action);
   
   QVector<Account *> accounts = SFLPhone::model()->getAccountList()->registeredAccounts();
   for (int i = 0 ; i < accounts.size() ; i++) {
      Account* account = accounts.at(i);
      QAction* action = new ActionSetAccountFirst(account, &menu);
      action->setChecked(account->getAccountId() == SFLPhone::model()->getPriorAccoundId());
      connect(action, SIGNAL(setFirst(Account *)), this  , SLOT(setAccountFirst(Account *)));
      menu.addAction(action);
   }
   menu.exec(event->globalPos());
}

void SFLPhoneView::editBeforeCall()
{
   qDebug() << "editBeforeCall";
   QString name;
   QString number;
        
   bool ok;
   QString newNumber = QInputDialog::getText(this, i18n("Edit before call"), QString(), QLineEdit::Normal, number, &ok);
   if(ok) {
      changeScreen(SCREEN_MAIN);
      Call* call = SFLPhone::model()->addDialingCall(name);
      call->appendText(newNumber);
      //callTreeModel->selectItem(addCallToCallList(call));
      action(call, CALL_ACTION_ACCEPT);
   }
}

void SFLPhoneView::setAccountFirst(Account * account)
{
   qDebug() << "setAccountFirst : " << (account ? account->getAlias() : QString()) << (account ? account->getAccountId() : QString());
   if(account) {
      SFLPhone::model()->setPriorAccountId(account->getAccountId());
   }
   else {
      SFLPhone::model()->setPriorAccountId(QString());
   }
   qDebug() << "Current account id" << SFLPhone::model()->getCurrentAccountId();
   updateStatusMessage();
}

void SFLPhoneView::configureSflPhone()
{
   ConfigurationDialog* configDialog = new ConfigurationDialog(this);
   configDialog->setModal(true);
   
   connect(configDialog, SIGNAL(changesApplied()),
           this,         SLOT(loadWindow()));
           
   //configDialog->reload();
   configDialog->show();
}

void SFLPhoneView::accountCreationWizard()
{
   if (!wizard) {
      wizard = new AccountWizard(this);
      wizard->setModal(false);
   }
   wizard->show();
}
   

void SFLPhoneView::accept()
{
   Call* call = callTreeModel->getCurrentItem();
   if(!call) {
      qDebug() << "Calling when no item is selected. Opening an item.";
      SFLPhone::model()->addDialingCall();
   }
   else {
      int state = call->getState();
      if(state == CALL_STATE_RINGING || state == CALL_STATE_CURRENT || state == CALL_STATE_HOLD || state == CALL_STATE_BUSY)
      {
         qDebug() << "Calling when item currently ringing, current, hold or busy. Opening an item.";
         SFLPhone::model()->addDialingCall();
      }
      else {
         action(call, CALL_ACTION_ACCEPT);
      }
   }
}

void SFLPhoneView::refuse()
{
   Call* call = callTreeModel->getCurrentItem();
   if(!call) {
      qDebug() << "Error : Hanging up when no item selected. Should not happen.";
   }
   else {
      action(call, CALL_ACTION_REFUSE);
   }
}

void SFLPhoneView::hold()
{
   Call* call = callTreeModel->getCurrentItem();
   if(!call) {
      qDebug() << "Error : Holding when no item selected. Should not happen.";
   }
   else {
      action(call, CALL_ACTION_HOLD);
   }
}

void SFLPhoneView::transfer()
{
   Call* call = callTreeModel->getCurrentItem();
   if(!call) {
      qDebug() << "Error : Transfering when no item selected. Should not happen.";
   }
   else {
      action(call, CALL_ACTION_TRANSFER);
   }
}

void SFLPhoneView::record()
{
   Call* call = callTreeModel->getCurrentItem();
   if(!call) {
      qDebug() << "Error : Recording when no item selected. Should not happen.";
   }
   else {
      action(call, CALL_ACTION_RECORD);
   }
}

void SFLPhoneView::mailBox()
{
   Account * account = SFLPhone::model()->getCurrentAccount();
   QString mailBoxNumber = account->getAccountDetail(ACCOUNT_MAILBOX);
   Call * call = SFLPhone::model()->addDialingCall();
   call->appendText(mailBoxNumber);
   action(call, CALL_ACTION_ACCEPT);
}

void SFLPhoneView::on1_error(MapStringString details)
{
   qDebug() << "Signal : Daemon error : " << details;
}

void SFLPhoneView::on1_incomingCall(Call* call)
{
   qDebug() << "Signal : Incoming Call ! ID = " << call->getCallId();
   //Call* call = SFLPhone::model()->addIncomingCall(callID);
   

   //NEED_PORT
   changeScreen(SCREEN_MAIN);

   SFLPhone::app()->activateWindow  (      );
   SFLPhone::app()->raise           (      );
   SFLPhone::app()->setVisible      ( true );

   emit incomingCall(call);
}

void SFLPhoneView::on1_voiceMailNotify(const QString &accountID, int count)
{
   qDebug() << "Signal : VoiceMail Notify ! " << count << " new voice mails for account " << accountID;
}

void SFLPhoneView::on1_volumeChanged(const QString & /*device*/, double value)
{
   qDebug() << "Signal : Volume Changed !";
   if(! (toolButton_recVol->isChecked() && value == 0.0))
      updateRecordBar();
   if(! (toolButton_sndVol->isChecked() && value == 0.0))
      updateVolumeBar();
}

// void SFLPhoneView::on1_audioManagerChanged()
// {
//    qDebug() << "Signal : Audio Manager Changed !";
// 
//    updateVolumeControls();
// }

void SFLPhoneView::changeScreen(int screen)
{
   qDebug() << "changeScreen";
   updateWindowCallState();
   emit screenChanged(screen);
}

#include "SFLPhoneView.moc"
