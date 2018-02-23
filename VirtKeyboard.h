#ifndef VIRTKEYBOARD_H
#define VIRTKEYBOARD_H

#include <QWidget>
#include <QPushButton>
#include <QSize>
#include <QStackedWidget>
#include <QHBoxLayout>

///////////////////////////////////////////////////////////////////////////////

#define VK_KEYCHAR_HEADER       '$'
#define VK_KEYCHAR_CHARACTER    '#'
#define VK_KEYCHAR_WIDTH        '%'
#define VK_KEYCHAR_COMMAND      '&'
#define VK_KEYCHAR_COMMAND_TEXT ':'
#define VK_KEYCHAR_LINK         '@'
#define VK_KEYCHAR_PRESSED      '!'
#define VK_KEYCHAR_SPACER       '~'

// Header should contain alphanumeric characters only.

// Сommand should be equal to one of VirtKey::Command values. May be empty.

// Command text should not contain VK_KEYCHAR_WIDTH, VK_KEYCHAR_LINK, VK_KEYCHAR_PRESSED and space characters.
// All '_' characters in command text are replaced by spaces during parsing.

// VK_KEYCHAR_PRESSED, if present, should be the last character in key definition

#define VK_DEFAULT_LAYOUT \
    "$letters\n" \
    "#q #w #e #r #t #y #u #i #o #p\n" \
    "#a #s #d #f #g #h #j #k #l\n" \
    "&SHIFT:Shift@caps%1.624 #z #x #c #v #b #n #m &BKSPACE:BkSpace%1.625\n" \
    "&:?123@symbols1%1.624 #@ &SPACE%5.9 #. &ENTER:Enter%1.625\n" \
    "\n" \
    "$caps\n" \
    "#Q #W #E #R #T #Y #U #I #O #P\n" \
    "#A #S #D #F #G #H #J #K #L\n" \
    "&SHIFT:Shift@letters%1.624! #Z #X #C #V #B #N #M &BKSPACE:BkSpace%1.625\n" \
    "&:?123@symbols2%1.624 #@ &SPACE%5.9 #. &ENTER:Enter%1.625\n" \
    "\n" \
    "$symbols1\n" \
    "#1 #2 #3 #4 #5 #6 #7 #8 #9 #0\n" \
    "#@ ## #$ #% #€ #& #* #- #+\n" \
    "#( #) #! #\" #' #: #; #/ #? &BKSPACE:BkSp\n" \
    "&:ABC@letters%1.624 #= #_ &SPACE%3.46 #, #. &ENTER:Enter%1.625\n" \
    "\n" \
    "$symbols2\n" \
    "#1 #2 #3 #4 #5 #6 #7 #8 #9 #0\n" \
    "#@ ## #$ #% #€ #& #* #- #+\n" \
    "#( #) #! #\" #' #: #; #/ #? &BKSPACE:BkSp\n" \
    "&:ABC@caps%1.624 #= #_ &SPACE%3.46 #, #. &ENTER:Enter%1.625\n" \
    "\n"

///////////////////////////////////////////////////////////////////////////////

class VirtKeyData;

class VirtKey : public QPushButton
{
    Q_OBJECT

public:
    enum class Type { UNDEFINED = 0, CHARACTER, COMMAND, SPACER };

    enum class Command { UNDEFINED = 0, CUSTOM, SHIFT, SPACE, BKSPACE, ENTER };
    Q_ENUM(Command)

    explicit VirtKey(const VirtKeyData& data, QWidget* parent = nullptr);
    ~VirtKey() override;

    VirtKeyData* data() { return _data; }

private:
    VirtKeyData* _data = nullptr;
};

///////////////////////////////////////////////////////////////////////////////

class VirtKeyData
{
public:
    VirtKey::Type type = VirtKey::Type::UNDEFINED;
    VirtKey::Command command = VirtKey::Command::UNDEFINED;

    QString text;
    double widthFactor = 1.0;
    bool pressed = false;

    QString linkStr;
    QWidget* link = nullptr;
    QImage* image = nullptr;
};

///////////////////////////////////////////////////////////////////////////////

class VirtKeyboard : public QWidget
{
    Q_OBJECT

    typedef QWidget Base;

public:
    explicit VirtKeyboard(QWidget* parent = nullptr);
    ~VirtKeyboard() override;

    void loadLayout(const QString& layoutFilePath);
    void createLayout(QString layoutStr = "");

    void setKeySize(QSize keySize) { _keySize = keySize; }
    void setKeySize(int width, int height) { _keySize.setWidth(width); _keySize.setHeight(height); }

    void setImageShift(const QImage& imageShift)     { setImage(&_imageShift,   imageShift); }
    void setImageBkSpace(const QImage& imageBkSpace) { setImage(&_imageBkSpace, imageBkSpace); }
    void setImageEnter(const QImage& imageEnter)     { setImage(&_imageEnter,   imageEnter); }

private:
    void setImage(QImage** dst, const QImage& src);

    void parsePage(QStringList page);
    void parseLine(const QString& line, QHBoxLayout* lineLayout);
    void createKey(const VirtKeyData& data, QHBoxLayout* lineLayout);

Q_SIGNALS:
    void keyClicked(VirtKeyData key);

public Q_SLOTS:
    void onKeyClicked();

private:
    QSize _keySize = QSize(35, 35);
    int _padding = 9;

    QImage* _imageShift = nullptr;
    QImage* _imageBkSpace = nullptr;
    QImage* _imageEnter = nullptr;

    QStackedWidget* _stackedWidget = nullptr;
};

#endif // VIRTKEYBOARD_H
