/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

#include <test/libyul/ControlFlowSideEffectsTest.h>

#include <test/Common.h>
#include <test/libyul/Common.h>

#include <libyul/Object.h>
#include <libyul/ControlFlowSideEffects.h>
#include <libyul/ControlFlowSideEffectsCollector.h>
#include <libyul/backends/evm/EVMDialect.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::frontend::test;

namespace
{
string toString(Modality _modality)
{
	switch (_modality)
	{
	case Modality::Never: return "never";
	case Modality::Maybe: return "maybe";
	case Modality::Always: return "always";
	}
	return "INVALID";
}

string toString(ControlFlowSideEffects const& _sideEffects)
{
	return
		toString(_sideEffects.terminates) +
		" terminates, " +
		toString(_sideEffects.reverts) +
		" reverts, " +
		toString(_sideEffects.loops) +
		" loops";
}
}

ControlFlowSideEffectsTest::ControlFlowSideEffectsTest(string const& _filename):
	TestCase(_filename)
{
	m_source = m_reader.source();
	m_expectation = m_reader.simpleExpectations();
}

TestCase::TestResult ControlFlowSideEffectsTest::run(ostream& _stream, string const& _linePrefix, bool _formatted)
{
	Object obj;
	std::tie(obj.code, obj.analysisInfo) = yul::test::parse(m_source, false);
	if (!obj.code)
		BOOST_THROW_EXCEPTION(runtime_error("Parsing input failed."));

	std::map<YulString, ControlFlowSideEffects> sideEffects =
		ControlFlowSideEffectsCollector::sideEffectsOfFunctions(
			*obj.code,
			EVMDialect::strictAssemblyForEVMObjects(langutil::EVMVersion())
		);

	std::map<std::string, std::string> controlFlowSideEffectsStr;
	for (auto&& [fun, effects]: sideEffects)
		controlFlowSideEffectsStr[fun.str()] = toString(effects);

	m_obtainedResult.clear();
	for (auto&& [functionName, effect]: controlFlowSideEffectsStr)
		m_obtainedResult += functionName + ":" + effect + "\n";

	return checkResult(_stream, _linePrefix, _formatted);
}
