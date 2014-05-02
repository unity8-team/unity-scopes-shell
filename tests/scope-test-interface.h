/*
 * scope-test-interface.h
 *
 *  Created on: 30 Apr 2014
 *      Author: pete
 */

#ifndef SCOPE_TEST_INTERFACE_H_
#define SCOPE_TEST_INTERFACE_H_

#include <QObject>

class TestAdaptor;

class ScopeTestInterface: public QObject
{
Q_OBJECT
public:
    ScopeTestInterface();

    virtual ~ScopeTestInterface();

Q_SIGNALS:
    void Preview();
    void Search(const QString &query);
    void Start();
    void Stop();

protected:
    QScopedPointer<TestAdaptor> m_adaptor;
};

#endif /* SCOPE_TEST_INTERFACE_H_ */
