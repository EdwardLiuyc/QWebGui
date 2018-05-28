#ifndef ADDMAPSETTINGDLG_H
#define ADDMAPSETTINGDLG_H

#include <QDialog>
#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include "common_defines.h"

class BrowserLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit BrowserLineEdit(QWidget* parent = 0, bool with_button = true);

protected:
    void resizeEvent(QResizeEvent* event);

signals:
    void signalBrowse();

public slots:
    void slotOnBtnClicked()
    {
        emit signalBrowse();
    }

private:
    QToolButton* browser_btn_;
};



class AddMapSettingDlg : public QDialog
{
    Q_OBJECT
public:
    explicit AddMapSettingDlg( QWidget* parent = 0 );

    enum Contents
    {
        kName, kImageFilePath, kImageSettingFilePath, kContentCount
    };

    int32_t getSetting( MapSetting* setting );

protected:
    void resizeEvent(QResizeEvent* event);

    int32_t checkSetting();

public slots:
    void slotBrowseImageFile();
    void slotBrowseXmlFile();

private:
    QLabel* content_label_[kContentCount];
    BrowserLineEdit* browser_edit_[kContentCount];
    MapSetting map_setting_;
};

#endif // ADDMAPSETTINGDLG_H
