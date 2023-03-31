/**
 * @file setSpiderMonkeyException.cc
 * @author Caleb Aikens (caleb@distributive.network)
 * @brief Call this function whenever a JS_* function call fails in order to set an appropriate python exception (remember to also return NULL)
 * @version 0.1
 * @date 2023-02-28
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "include/modules/pythonmonkey/pythonmonkey.hh"
#include "include/setSpiderMonkeyException.hh"
#include "include/StrType.hh"

#include <jsapi.h>

#include <codecvt>
#include <locale>

#include <Python.h>

void setSpiderMonkeyException(JSContext *cx) {
  if (!JS_IsExceptionPending(cx)) {
    PyErr_SetString(SpiderMonkeyError, "Spidermonkey failed, but spidermonkey did not set an exception.");
    return;
  }
  JS::ExceptionStack exceptionStack(cx);
  if (!JS::GetPendingExceptionStack(cx, &exceptionStack)) {
    PyErr_SetString(SpiderMonkeyError, "Spidermonkey set an exception, but was unable to retrieve it.");
    return;
  }
  JS::ErrorReportBuilder reportBuilder(cx);
  if (!reportBuilder.init(cx, exceptionStack, JS::ErrorReportBuilder::WithSideEffects /* may call the `toString` method if an object is thrown */)) {
    PyErr_SetString(SpiderMonkeyError, "Spidermonkey set an exception, but could not initialize the error report.");
    return;
  }

  /**
   * the resulting python error string will be in the format:
   * "Error in file <filename>, on line <lineno>:
   * <offending line of code if relevant, nothing if not>
   * <if offending line is present, then a '^' pointing to the relevant token>
   * <spidermonkey error message>
   * Stack Track:
   * <stack trace>"
   *
   */
  std::stringstream outStrStream;

  JSErrorReport *errorReport = reportBuilder.report();
  if (errorReport) {
    std::string offsetSpaces(errorReport->tokenOffset(), ' '); // number of spaces equal to tokenOffset
    std::string linebuf; // the offending JS line of code (can be empty)

    outStrStream << "Error in file " << errorReport->filename << ", on line " << errorReport->lineno << ":\n";
    if (errorReport->linebuf()) {
      std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
      std::u16string u16linebuf(errorReport->linebuf());
      linebuf = convert.to_bytes(u16linebuf);
    }
    if (linebuf.size()) {
      outStrStream << linebuf << "\n";
      outStrStream << offsetSpaces << "^\n";
    }
  }

  // print out the SpiderMonkey error message
  outStrStream << reportBuilder.toStringResult().c_str() << "\n";

  JS::HandleObject stackObj = exceptionStack.stack();
  if (stackObj) { // stack can be null
    JS::RootedString stackStr(cx);
    BuildStackString(cx, nullptr, stackObj, &stackStr, /* indent */ 2, js::StackFormat::SpiderMonkey);
    outStrStream << "Stack Trace: \n" << StrType(cx, stackStr).getValue();
  }

  PyErr_SetString(SpiderMonkeyError, outStrStream.str().c_str());
}