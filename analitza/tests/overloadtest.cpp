// SPDX-License-Identifier: GPL-2.0-or-later
#include <QTest>
#include <analitza/analyzer.h>
#include <analitza/value.h>
#include <analitza/variables.h>

class OverloadTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void namedCallsAndScopes()
    {
        auto variables = QSharedPointer<Analitza::Variables>::create();
        variables->modify(QStringLiteral("f"), Analitza::Expression(QStringLiteral("(x,y)->x-13*y")));
        variables->setFunctionOverload(QStringLiteral("f"), Analitza::Expression(QStringLiteral("x->x/13")));
        Analitza::Analyzer analyzer(variables);
        for (const auto &source : {QStringLiteral("f(12)"), QStringLiteral("2*f(6)"), QStringLiteral("(x->f(x))(12)")}) {
            analyzer.setExpression(Analitza::Expression(source));
            QVERIFY2(analyzer.isCorrect(), qPrintable(analyzer.errors().join(QLatin1Char('\n'))));
            const auto value = analyzer.calculate();
            QVERIFY(analyzer.isCorrect());
            QCOMPARE(value.toReal().value(), 12./13.);
            analyzer.setExpression(Analitza::Expression(source));
            QCOMPARE(analyzer.evaluate().toReal().value(), 12./13.);
        }
        analyzer.setExpression(Analitza::Expression(QStringLiteral("f(12,1)")));
        QCOMPARE(analyzer.calculate().toReal().value(), -1.);
        analyzer.setExpression(Analitza::Expression(QStringLiteral("(f->f(12))(z->z+1)")));
        QVERIFY2(analyzer.isCorrect(), qPrintable(analyzer.errors().join(QLatin1Char('\n'))));
        QCOMPARE(analyzer.calculate().toReal().value(), 13.);
        analyzer.setExpression(Analitza::Expression(QStringLiteral("f()")));
        QVERIFY(!analyzer.isCorrect());
    }
    void removalDoesNotLeaveAnOverload()
    {
        Analitza::Variables variables;
        variables.modify(QStringLiteral("f"), Analitza::Expression(QStringLiteral("(x,y)->x-y")));
        variables.setFunctionOverload(QStringLiteral("f"), Analitza::Expression(QStringLiteral("x->x")));
        QCOMPARE(variables.remove(QStringLiteral("f")), 1);
        QVERIFY(!variables.contains(QStringLiteral("f")));
        QCOMPARE(variables.remove(QStringLiteral("f")), 0);
        variables.modify(QStringLiteral("f"), Analitza::Expression(QStringLiteral("(x,y)->x+y")));
        QVERIFY(!variables.functionOverload(QStringLiteral("f"), 1));
    }
    void copyRenameAndRedefine()
    {
        Analitza::Variables original;
        original.modify(QStringLiteral("f"), Analitza::Expression(QStringLiteral("(x,y)->x-y")));
        original.setFunctionOverload(QStringLiteral("f"), Analitza::Expression(QStringLiteral("x->x")));
        auto copy = QSharedPointer<Analitza::Variables>::create(original);
        original.modify(QStringLiteral("f"), 42.);
        QVERIFY(!original.functionOverload(QStringLiteral("f"), 1));
        copy->rename(QStringLiteral("f"), QStringLiteral("g"));
        QVERIFY(!copy->functionOverload(QStringLiteral("f"), 1));
        Analitza::Analyzer analyzer(copy);
        analyzer.setExpression(Analitza::Expression(QStringLiteral("g(3)")));
        QCOMPARE(analyzer.calculate().toReal().value(), 3.);
        copy->modify(QStringLiteral("g"), Analitza::Expression(QStringLiteral("(x,y)->x+y")));
        analyzer.setExpression(Analitza::Expression(QStringLiteral("g(3)")));
        QVERIFY(!analyzer.isCorrect());
    }
};
QTEST_GUILESS_MAIN(OverloadTest)
#include "overloadtest.moc"
