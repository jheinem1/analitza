// SPDX-License-Identifier: GPL-2.0-or-later
#include <analitza/analyzer.h>
#include <QTest>

class DependenciesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void inferredArguments()
    {
        Analitza::Analyzer analyzer;
        analyzer.setExpression(Analitza::Expression(QStringLiteral("x-2*y")));
        QCOMPARE(analyzer.dependenciesToLambda().bvarList(), QStringList({QStringLiteral("x"), QStringLiteral("y")}));
    }
    void explicitArguments()
    {
        Analitza::Analyzer analyzer;
        analyzer.setExpression(Analitza::Expression(QStringLiteral("(y,x)->x-2*y")));
        QCOMPARE(analyzer.dependenciesToLambda().bvarList(), QStringList({QStringLiteral("y"), QStringLiteral("x")}));
    }
};
QTEST_GUILESS_MAIN(DependenciesTest)
#include "dependenciestest.moc"
