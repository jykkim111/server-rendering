#pragma once

#ifndef UI_OTFPRESETVIEWER_H
#define UI_OTFPRESETVIEWER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class UI_OTFPresetViewer
{
public:
    QHBoxLayout* horizontalLayout_2;
    QGraphicsView* m_view;
    QVBoxLayout* verticalLayout;
    QListView* m_listPreview;
    QPushButton* m_btnSelect;
    QPushButton* m_btnRename;
    QHBoxLayout* horizontalLayout;
    QPushButton* m_btnAdd;
    QPushButton* m_btnDelete;
    QSpacerItem* verticalSpacer;

    void setupUi(QWidget* OTFPresetViewer)
    {
        if (OTFPresetViewer->objectName().isEmpty())
            OTFPresetViewer->setObjectName(QStringLiteral("OTFPresetViewer"));
        OTFPresetViewer->resize(300, 454);
        horizontalLayout_2 = new QHBoxLayout(OTFPresetViewer);
        horizontalLayout_2->setSpacing(2);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(2, 2, 2, 2);
        //m_view = new QGraphicsView(OTFPresetViewer);
        //m_view->setObjectName(QStringLiteral("m_view"));
        //QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        //sizePolicy.setHorizontalStretch(0);
        //sizePolicy.setVerticalStretch(0);
        //sizePolicy.setHeightForWidth(m_view->sizePolicy().hasHeightForWidth());
        //m_view->setSizePolicy(sizePolicy);
        //m_view->setMinimumSize(QSize(450, 450));
        //m_view->setMaximumSize(QSize(450, 450));
        //m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        //m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        //QBrush brush(QColor(0, 0, 0, 255));
        //brush.setStyle(Qt::SolidPattern);
        //m_view->setBackgroundBrush(brush);
        //
        //horizontalLayout_2->addWidget(m_view);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, -1, -1, -1);
        m_listPreview = new QListView(OTFPresetViewer);
        m_listPreview->setObjectName(QStringLiteral("m_listPreview"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(m_listPreview->sizePolicy().hasHeightForWidth());
        m_listPreview->setSizePolicy(sizePolicy1);
        m_listPreview->setMinimumSize(QSize(250, 330));
        m_listPreview->setMaximumSize(QSize(290, 350));

        verticalLayout->addWidget(m_listPreview);

        m_btnSelect = new QPushButton(OTFPresetViewer);
        m_btnSelect->setObjectName(QStringLiteral("m_btnSelect"));

        verticalLayout->addWidget(m_btnSelect);

        m_btnRename = new QPushButton(OTFPresetViewer);
        m_btnRename->setObjectName(QStringLiteral("m_btnRename"));

        verticalLayout->addWidget(m_btnRename);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(-1, 0, -1, -1);
        m_btnAdd = new QPushButton(OTFPresetViewer);
        m_btnAdd->setObjectName(QStringLiteral("m_btnAdd"));

        horizontalLayout->addWidget(m_btnAdd);

        m_btnDelete = new QPushButton(OTFPresetViewer);
        m_btnDelete->setObjectName(QStringLiteral("m_btnDelete"));

        horizontalLayout->addWidget(m_btnDelete);


        verticalLayout->addLayout(horizontalLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        horizontalLayout_2->addLayout(verticalLayout);


        retranslateUi(OTFPresetViewer);

        QMetaObject::connectSlotsByName(OTFPresetViewer);
    } // setupUi

    void retranslateUi(QWidget* OTFPresetViewer)
    {
        OTFPresetViewer->setWindowTitle(QApplication::translate("OTFPresetViewer", "CAzOTFPresetViewer", 0));
        m_btnSelect->setText(QApplication::translate("OTFPresetViewer", "Select", 0));
        m_btnRename->setText(QApplication::translate("OTFPresetViewer", "Rename", 0));
        m_btnAdd->setText(QApplication::translate("OTFPresetViewer", "Add", 0));
        m_btnDelete->setText(QApplication::translate("OTFPresetViewer", "Delete", 0));
    } // retranslateUi

};

namespace Ui {
    class OTFPresetViewer : public UI_OTFPresetViewer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OTFPRESETVIEWER_H
