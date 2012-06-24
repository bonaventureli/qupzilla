/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
#include "adblockdialog.h"
#include "adblockmanager.h"
#include "adblocksubscription.h"
#include "adblocktreewidget.h"
#include "adblockaddsubscriptiondialog.h"
#include "mainapplication.h"

#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>

AdBlockDialog::AdBlockDialog(QWidget* parent)
    : QDialog(parent)
    , m_manager(AdBlockManager::instance())
    , m_currentTreeWidget(0)
    , m_currentSubscription(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setupUi(this);

    adblockCheckBox->setChecked(m_manager->isEnabled());

    QMenu* menu = new QMenu(buttonMenu);
    m_actionAddRule = menu->addAction(tr("Add Rule"), this, SLOT(addRule()));
    m_actionRemoveRule = menu->addAction(tr("Remove Rule"), this, SLOT(removeRule()));
    menu->addSeparator();
    m_actionAddSubscription = menu->addAction(tr("Add Subscription"), this, SLOT(addSubscription()));
    m_actionRemoveSubscription = menu->addAction(tr("Remove Subscription"), this, SLOT(removeSubscription()));
    menu->addAction(tr("Update Subscriptions"), m_manager, SLOT(updateAllSubscriptions()));
    menu->addSeparator();
    menu->addAction(tr("Learn about writing rules..."), this, SLOT(learnAboutRules()));

    buttonMenu->setMenu(menu);
    connect(menu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowMenu()));

    connect(adblockCheckBox, SIGNAL(toggled(bool)), m_manager, SLOT(setEnabled(bool)));
    connect(search, SIGNAL(textChanged(QString)), this, SLOT(filterString(QString)));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));

    foreach(AdBlockSubscription * subscription, m_manager->subscriptions()) {
        AdBlockTreeWidget* tree = new AdBlockTreeWidget(subscription, tabWidget);
        tabWidget->addTab(tree, subscription->title());
    }

    buttonBox->setFocus();
}

void AdBlockDialog::addRule()
{
    m_currentTreeWidget->addRule();
}

void AdBlockDialog::removeRule()
{
    m_currentTreeWidget->removeRule();
}

void AdBlockDialog::addSubscription()
{
    AdBlockAddSubscriptionDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString title = dialog.title();
    QString url = dialog.url();

    if (AdBlockSubscription* subscription = m_manager->addSubscription(title, url)) {
        AdBlockTreeWidget* tree = new AdBlockTreeWidget(subscription, tabWidget);
        int index = tabWidget->insertTab(tabWidget->count() - 1, tree, subscription->title());

        tabWidget->setCurrentIndex(index);
    }
}

void AdBlockDialog::removeSubscription()
{
    if (m_manager->removeSubscription(m_currentSubscription)) {
        delete m_currentTreeWidget;
    }
}

void AdBlockDialog::currentChanged(int index)
{
    if (index != -1) {
        m_currentTreeWidget = qobject_cast<AdBlockTreeWidget*>(tabWidget->widget(index));
        m_currentSubscription = m_currentTreeWidget->subscription();

        if (!search->text().isEmpty()) {
            filterString(search->text());
        }
    }
}

void AdBlockDialog::filterString(const QString &string)
{
    m_currentTreeWidget->filterString(string);
}

void AdBlockDialog::aboutToShowMenu()
{
    bool subscriptionEditable = m_currentSubscription->canEditRules();
    bool subscriptionRemovable = m_currentSubscription->canBeRemoved();

    m_actionAddRule->setEnabled(subscriptionEditable);
    m_actionRemoveRule->setEnabled(subscriptionEditable);
    m_actionRemoveSubscription->setEnabled(subscriptionRemovable);
}

void AdBlockDialog::learnAboutRules()
{
    mApp->addNewTab(QUrl("http://adblockplus.org/en/filters"));
}
