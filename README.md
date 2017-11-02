# VirtKeyboard
Qt virtual keyboard widget

Example of usage.

```
//...

QHBoxLayout* layout = new QHBoxLayout();
layout->setContentsMargins(0, 0, 0, 0);
_ui->frameKeyboard->setLayout(layout);

_keyboard = new VirtKeyboard(_ui->frameKeyboard);
_keyboard->setImageShift(QImage(":/resources/shift.png"));
_keyboard->setImageBkSpace(QImage(":/resources/backspace.png"));
_keyboard->setImageEnter(QImage(":/resources/enter.png"));
_keyboard->loadLayout(QCoreApplication::applicationDirPath() + "/" + language + ".layout");

layout->addWidget(_keyboard);

connect(_keyboard, &VirtKeyboard::keyClicked, this, &MainWindow::onVirtKeyClicked);

//...

void MainWindow::onVirtKeyClicked(VirtKeyData key)
{
    QLineEdit* edit = dynamic_cast<QLineEdit*>(QApplication::focusWidget());
    if (edit == nullptr) return;
    
    if (key.type == VirtKey::Type::CHARACTER)
    {
        edit->insert(key.text);
    }
    else if (key.type == VirtKey::Type::COMMAND)
    {
        switch (key.command)
        {
            case VirtKey::Command::SPACE:   edit->insert(" ");        break;
            case VirtKey::Command::BKSPACE: edit->backspace();        break;
            case VirtKey::Command::ENTER:   this->activateNext(edit); break;
        }
    }
}
```
