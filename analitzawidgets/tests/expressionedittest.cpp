// SPDX-License-Identifier: GPL-2.0-or-later
#include "../../analitzagui/contrastcolors_p.h"
#include <QApplication>
#include <QSignalSpy>
#include <QTest>
#include <QTextBlock>
#include <QTextLayout>
#include <analitza/analyzer.h>
#include <expressionedit.h>

class ExpressionEditTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void selectedCompletion_data()
    {
        QTest::addColumn<int>("key");
        QTest::newRow("return") << int(Qt::Key_Return);
        QTest::newRow("enter") << int(Qt::Key_Enter);
        QTest::newRow("tab") << int(Qt::Key_Tab);
    }
    void selectedCompletion()
    {
        QFETCH(int, key);
        Analitza::Analyzer analyzer;
        analyzer.insertVariable(QStringLiteral("ans"), Analitza::Expression(QStringLiteral("42")));
        Analitza::ExpressionEdit edit;
        edit.setAnalitza(&analyzer);
        edit.show();
        edit.setFocus();
        QSignalSpy submitted(&edit, &Analitza::ExpressionEdit::returnPressed);
        QTest::keyClicks(&edit, "an");
        auto completer = edit.findChild<QCompleter *>();
        QVERIFY(completer);
        auto popup = completer->popup();
        QTRY_VERIFY(popup->isVisible());
        QVERIFY(completer->completionCount() >= 2);
        for (int i = 0; i < 3 && popup->currentIndex().data().toString() != QStringLiteral("ans"); ++i)
            QTest::keyClick(popup, Qt::Key_Down);
        QTRY_COMPARE(popup->currentIndex().data().toString(), QStringLiteral("ans"));
        QTest::keyClick(&edit, Qt::Key(key));
        QCOMPARE(edit.text(), QStringLiteral("ans"));
        QCOMPARE(submitted.count(), 0);
        QVERIFY(!popup->isVisible());
    }
    void highlightingContrast_data()
    {
        QTest::addColumn<QColor>("base");
        QTest::addColumn<QColor>("text");
        QTest::newRow("dark") << QColor(QStringLiteral("#141618")) << QColor(QStringLiteral("#eff0f1"));
        QTest::newRow("light") << QColor(Qt::white) << QColor(Qt::black);
    }
    void highlightingContrast()
    {
        QFETCH(QColor, base);
        QFETCH(QColor, text);
        const auto original = qApp->palette();
        auto palette = original;
        palette.setColor(QPalette::Base, base);
        palette.setColor(QPalette::Text, text);
        qApp->setPalette(palette);
        Analitza::ExpressionEdit edit;
        edit.setText(QStringLiteral("sin(x)+5+\"abc\""));
        auto highlighter = edit.document()->findChild<QSyntaxHighlighter *>();
        QVERIFY(highlighter);
        highlighter->rehighlight();
        const auto formats = edit.document()->firstBlock().layout()->formats();
        QVERIFY(!formats.isEmpty());
        for (const auto &range : formats) {
            if (!range.format.hasProperty(QTextFormat::ForegroundBrush))
                continue;
            const double foreground = Analitza::relativeLuminance(range.format.foreground().color());
            const double background = Analitza::relativeLuminance(base);
            QVERIFY((std::max(foreground, background) + 0.05) / (std::min(foreground, background) + 0.05) >= 4.5);
        }
        edit.setCorrect(false);
        QVERIFY(edit.palette().base().color().lightnessF() < 0.3 || base == QColor(Qt::white));
        qApp->setPalette(original);
    }
    void historyAndSubmit()
    {
        Analitza::ExpressionEdit edit;
        edit.show();
        edit.setFocus();
        QSignalSpy submitted(&edit, &Analitza::ExpressionEdit::returnPressed);
        QTest::keyClicks(&edit, "1+2");
        QTest::keyClick(&edit, Qt::Key_Return);
        QCOMPARE(submitted.count(), 1);
        edit.clear();
        QTest::keyClick(&edit, Qt::Key_Up);
        QCOMPARE(edit.text(), QStringLiteral("1+2"));
    }
};
QTEST_MAIN(ExpressionEditTest)
#include "expressionedittest.moc"
