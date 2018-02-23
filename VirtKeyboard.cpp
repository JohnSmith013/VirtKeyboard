#include "VirtKeyboard.h"

#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QDebug>
#include <QMetaEnum>

///////////////////////////////////////////////////////////////////////////////

VirtKey::VirtKey(const VirtKeyData& data, QWidget* parent) : QPushButton(parent)
{
    _data = new VirtKeyData();
    *_data = data;
}

VirtKey::~VirtKey()
{
    delete _data;
}

///////////////////////////////////////////////////////////////////////////////

VirtKeyboard::VirtKeyboard(QWidget* parent) : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);

    _stackedWidget = new QStackedWidget(this);

    layout->addWidget(_stackedWidget);

    //_stackedWidget->setFrameStyle(QFrame::Plain);
    //_stackedWidget->setFrameShape(QFrame::Box);
}

VirtKeyboard::~VirtKeyboard()
{
    delete _imageShift;
    delete _imageBkSpace;
    delete _imageEnter;
}

void VirtKeyboard::setImage(QImage** dst, const QImage& src)
{
    delete *dst;
    *dst = new QImage(src);
}

void VirtKeyboard::loadLayout(const QString& layoutFilePath)
{
    QString layout;

    if (QFile::exists(layoutFilePath))
    {
        QFile file(layoutFilePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream ts(&file);
            ts.setCodec("UTF-8");
            layout = ts.readAll();
        }
    }

    this->createLayout(layout);
}

void VirtKeyboard::createLayout(QString layoutStr)
{
    QWidget* widget = nullptr;
    while ((widget = _stackedWidget->widget(0)) != nullptr)
    {
        _stackedWidget->removeWidget(widget);
        delete widget;
    }

    if (layoutStr.isEmpty()) layoutStr = VK_DEFAULT_LAYOUT;

    QStringList layout = layoutStr.split("\n");

    QList<QStringList> pages;
    QStringList page;

    for (QString line : layout)
    {
        line = line.simplified();

        if (line.isEmpty()) continue;

        if (line.startsWith(VK_KEYCHAR_HEADER))
        {
            if (!page.isEmpty())
            {
                pages.append(page);
                page.clear();
            }
        }

        page.append(line);
    }
    if (!page.isEmpty()) pages.append(page);

    for (const QStringList& page : pages) this->parsePage(page);

    if (_stackedWidget->count() > 0) _stackedWidget->setCurrentIndex(0);
}

void VirtKeyboard::parsePage(QStringList page)
{
    if (page.isEmpty()) return;

    QString header = page[0];
    header = header.simplified();

    if (!header.startsWith(VK_KEYCHAR_HEADER)) return;

    header = header.right(header.length()-1);
    page.removeFirst();

    QWidget* pageWidget = new QWidget();
    pageWidget->setObjectName(header);

    QVBoxLayout* pageLayout = new QVBoxLayout();
    pageLayout->setContentsMargins(_padding, _padding, _padding, _padding);
    pageLayout->setSpacing(_padding);
    pageWidget->setLayout(pageLayout);

    QSpacerItem* verticalSpacer1 = new QSpacerItem(_padding, _padding, QSizePolicy::Minimum, QSizePolicy::Expanding);
    pageLayout->addItem(verticalSpacer1);

    for (const QString& line : page)
    {
        QHBoxLayout* lineLayout = new QHBoxLayout();
        pageLayout->addLayout(lineLayout);

        this->parseLine(line, lineLayout);
    }

    QSpacerItem* verticalSpacer2 = new QSpacerItem(_padding, _padding, QSizePolicy::Minimum, QSizePolicy::Expanding);
    pageLayout->addItem(verticalSpacer2);

    _stackedWidget->addWidget(pageWidget);
}

void VirtKeyboard::parseLine(const QString& line, QHBoxLayout* lineLayout)
{
    if (line.isEmpty() || (lineLayout == nullptr)) return;

    QSpacerItem* horizontalSpacer = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    lineLayout->addItem(horizontalSpacer);

    QStringList keys = line.simplified().split(" ");

    for (QString key : keys)
    {
        VirtKeyData data;

        if (key.startsWith(VK_KEYCHAR_CHARACTER))
            data.type = VirtKey::Type::CHARACTER;
        else if (key.startsWith(VK_KEYCHAR_COMMAND))
            data.type = VirtKey::Type::COMMAND;
        else if (key.startsWith(VK_KEYCHAR_SPACER))
            data.type = VirtKey::Type::SPACER;
        else continue;

        if (data.type == VirtKey::Type::CHARACTER)
        {
            if (key.length() > 1) data.text = key[1];
        }
        else if (data.type == VirtKey::Type::COMMAND)
        {
            data.command = VirtKey::Command::CUSTOM;

            int commandStartPos = 1;
            int commandEndPos = commandStartPos;
            for (; commandEndPos < key.length(); commandEndPos++)
            {
                QChar ch = key[commandEndPos];
                if (!ch.isLetter() && !ch.isDigit()) break;
            }

            if (commandStartPos < commandEndPos)
            {
                QString commandStr = key.mid(commandStartPos, commandEndPos - commandStartPos);

                QMetaEnum metaEnum = QMetaEnum::fromType<VirtKey::Command>();
                int commandInt = metaEnum.keyToValue(commandStr.toStdString().c_str());
                if (commandInt != -1) data.command = (VirtKey::Command)commandInt;
            }

            int commandTextStartPos = key.indexOf(VK_KEYCHAR_COMMAND_TEXT, commandEndPos) + 1;
            if (commandTextStartPos > commandEndPos)
            {
                int commandTextEndPos = commandTextStartPos;
                for (; commandTextEndPos < key.length(); commandTextEndPos++)
                {
                    QChar ch = key[commandTextEndPos];
                    if ((ch == VK_KEYCHAR_LINK) || (ch == VK_KEYCHAR_WIDTH) || (ch == VK_KEYCHAR_PRESSED)) break;
                }

                if (commandTextStartPos < commandTextEndPos)
                {
                    QString commandText = key.mid(commandTextStartPos, commandTextEndPos - commandTextStartPos);
                    commandText = commandText.replace("_", " ");
                    data.text = commandText;
                }
            }

            int linkStartPos = key.lastIndexOf(VK_KEYCHAR_LINK) + 1;
            if (linkStartPos > 1)
            {
                int linkEndPos = linkStartPos;
                for (; linkEndPos < key.length(); linkEndPos++)
                {
                    QChar ch = key[linkEndPos];
                    if (!ch.isLetter() && !ch.isDigit()) break;
                }

                if (linkStartPos < linkEndPos)
                {
                    data.linkStr = key.mid(linkStartPos, linkEndPos - linkStartPos);
                }
            }
        }

        int widthStartPos = key.lastIndexOf(VK_KEYCHAR_WIDTH);
        if (((data.type == VirtKey::Type::SPACER) && (widthStartPos == 1)) ||
            ((data.type != VirtKey::Type::SPACER) && (widthStartPos > 1)))
        {
            QString widthValue;
            widthStartPos++;
            int widthEndPos = widthStartPos;
            for (; widthEndPos < key.length(); widthEndPos++)
            {
                QChar ch = key[widthEndPos];
                if (!ch.isDigit() && (ch != '.')) break;
            }

            if (widthStartPos < widthEndPos)
            {
                data.widthFactor = key.mid(widthStartPos, widthEndPos - widthStartPos).toDouble();
                if (data.widthFactor <= 0.0) data.widthFactor = 1.0;
            }
        }

        if (key.endsWith(VK_KEYCHAR_PRESSED))
        {
            data.pressed = true;
            if ((data.type == VirtKey::Type::CHARACTER) && (key.length() == 2)) data.pressed = false;
        }

        this->createKey(data, lineLayout);
    }

    horizontalSpacer = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    lineLayout->addItem(horizontalSpacer);
}

void VirtKeyboard::createKey(const VirtKeyData& data, QHBoxLayout* lineLayout)
{
    if (data.type == VirtKey::Type::UNDEFINED) return;
    if (lineLayout == nullptr) return;

    QWidget* newWidget = nullptr;

    if ((data.type == VirtKey::Type::CHARACTER) || (data.type == VirtKey::Type::COMMAND))
    {
        VirtKey* key = new VirtKey(data);
        QImage* image = nullptr;

        if (data.type == VirtKey::Type::COMMAND)
        {
            switch (data.command)
            {
                case VirtKey::Command::SHIFT:   if (_imageShift != nullptr)   image = _imageShift;   break;
                case VirtKey::Command::BKSPACE: if (_imageBkSpace != nullptr) image = _imageBkSpace; break;
                case VirtKey::Command::ENTER:   if (_imageEnter != nullptr)   image = _imageEnter;   break;
            }
        }

        if (image != nullptr) key->setIcon(QPixmap::fromImage(*image));
        else                  key->setText(data.text == "&" ? "&&" : data.text);

        if (data.pressed) { key->setCheckable(true); key->setChecked(true); }

        key->setFocusPolicy(Qt::FocusPolicy::NoFocus);

        connect(key, &VirtKey::clicked, this, &VirtKeyboard::onKeyClicked);
        newWidget = key;
    }
    else if (data.type == VirtKey::Type::SPACER)
    {
        QWidget* spacer = new QWidget();
        newWidget = spacer;
    }

    if (newWidget == nullptr) return;

    QSize size(_keySize.width() * data.widthFactor, _keySize.height());
    newWidget->setFixedSize(size);

    lineLayout->addWidget(newWidget);
}

void VirtKeyboard::onKeyClicked()
{
    VirtKey* key = dynamic_cast<VirtKey*>(this->sender());
    if (key == nullptr) return;

    if (key->data()->pressed) key->setChecked(true);

    QString linkStr = key->data()->linkStr;
    if (!linkStr.isEmpty() && (key->data()->link == nullptr))
    {
        key->data()->link = this->findChild<QWidget*>(linkStr);
    }
    if (key->data()->link != nullptr)
    {
        _stackedWidget->setCurrentWidget(key->data()->link);
    }

    Q_EMIT keyClicked(*(key->data()));
}
