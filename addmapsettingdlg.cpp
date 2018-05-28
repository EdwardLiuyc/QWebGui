#include "addmapsettingdlg.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>

BrowserLineEdit::BrowserLineEdit(QWidget* parent, bool with_button)
        : QLineEdit(parent)
        , browser_btn_( new QToolButton(this) )
{
    this->setFont( QFont("SIMHEI", 10, QFont::Bold) );
    browser_btn_->setText("...");
    connect(browser_btn_, SIGNAL(clicked()), this, SLOT(slotOnBtnClicked()));
    if( !with_button )
        browser_btn_->hide();
}

void BrowserLineEdit::resizeEvent(QResizeEvent *event)
{
    int32_t edit_wth = this->width();
    int32_t btn_wdt = 20;
    int32_t btn_left = edit_wth - btn_wdt;
    if( btn_left < 0 )
    {
        btn_left = 0;
        btn_wdt = edit_wth;
    }
    browser_btn_->setGeometry( btn_left, 0, btn_wdt, this->height());

    QLineEdit::resizeEvent( event );
}

AddMapSettingDlg::AddMapSettingDlg(QWidget *parent)
    : QDialog( parent )
{
    resize( 540, 320 );
    setMaximumSize( 540, 320 );
    setMinimumSize( 540, 320 );

    QString content_strs[kContentCount] = {"Name","Image File Path","Xml File Path"};
    for( int i = 0; i < kContentCount; ++i )
    {
        content_label_[i] = new QLabel( this );
        content_label_[i]->setText(content_strs[i]);
        content_label_[i]->setFont( SYSTEM_UI_FONT_10_BOLD );
        content_label_[i]->adjustSize();

        if(i == kName)
            browser_edit_[i] = new BrowserLineEdit( this, false );
        else
            browser_edit_[i] = new BrowserLineEdit( this, true );
    }

    connect( browser_edit_[kImageFilePath], SIGNAL(signalBrowse()), this, SLOT(slotBrowseImageFile()));
    connect( browser_edit_[kImageSettingFilePath], SIGNAL(signalBrowse()), this, SLOT(slotBrowseXmlFile()));

    QString operation_str[kOperationCount] = {"Ok", "Cancel"};
    for( int i = 0; i < kOperationCount; ++i )
    {
        operation_btns_[i] = new QPushButton( this );
        operation_btns_[i]->setText( operation_str[i] );
        operation_btns_[i]->setFont( SYSTEM_UI_FONT_10_BOLD );
        operation_btns_[i]->setFocusPolicy(Qt::NoFocus);
    }
    QObject::connect( operation_btns_[kOk], SIGNAL(clicked()), this, SLOT(slotOkBtnClicked()));
    QObject::connect( operation_btns_[kCancel], SIGNAL(clicked()), this, SLOT(slotCancelBtnClicked()));
}

void AddMapSettingDlg::resizeEvent(QResizeEvent *event)
{
    int32_t view_wdt = this->width();

    int32_t gap_left = 60;
    int32_t gap_top = 80;
    int32_t lbl_hgt = 30;
    int32_t gap_hgt = 10;
    int32_t lbl_wdt = content_label_[kImageFilePath]->width();

    int32_t edit_left = gap_left + lbl_wdt + 10;
    int32_t edit_wdt = view_wdt - edit_left - gap_left;
    for( int i = 0; i < kContentCount; ++i )
    {
        content_label_[i]->setGeometry( gap_left, gap_top + i*(lbl_hgt+gap_hgt), lbl_wdt, lbl_hgt);
        browser_edit_[i]->setGeometry(edit_left, gap_top + i*(lbl_hgt+gap_hgt), edit_wdt, lbl_hgt);
    }

    int32_t btn_gap = 20;
    int32_t btn_wth = ( edit_wdt - btn_gap) / 2;
    for( int i = 0; i < kOperationCount; ++i )
        operation_btns_[i]->setGeometry( view_wdt - gap_left - (kOperationCount-i)*(btn_wth+btn_gap) + btn_gap,
                                         gap_top + 3*(lbl_hgt+gap_hgt),
                                         btn_wth, lbl_hgt );

    QDialog::resizeEvent( event );
}

void AddMapSettingDlg::slotBrowseImageFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), ".", tr("Image Files(*.pgm *.png)"));
    browser_edit_[kImageFilePath]->setText( path );
}

void AddMapSettingDlg::slotBrowseXmlFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open Xml File"), ".", tr("Image Files(*.xml *.XML)"));
    browser_edit_[kImageSettingFilePath]->setText( path );
}

void AddMapSettingDlg::slotOkBtnClicked()
{
    map_setting_.name_ = browser_edit_[kName]->text();
    map_setting_.image_file_name_ = browser_edit_[kImageFilePath]->text();
    map_setting_.image_setting_file_name_ = browser_edit_[kImageSettingFilePath]->text();
    if( checkSetting() < 0 )
    {
        qDebug() << "not good setting!";
        QMessageBox::critical(NULL, "critical", "The setting is not complete!", QMessageBox::Yes , QMessageBox::Yes);
    }
    else
        accept();
}

void AddMapSettingDlg::slotCancelBtnClicked()
{
    reject();
}


int32_t AddMapSettingDlg::getSetting(MapSetting *setting)
{
    if( checkSetting() < 0 )
        return -1;

    *setting = map_setting_;
    return 0;
}

int32_t AddMapSettingDlg::checkSetting()
{
    if( map_setting_.name_.isEmpty()
            || map_setting_.image_file_name_.isEmpty()
            || map_setting_.image_setting_file_name_.isEmpty() )
        return -1;

    return 0;
}
