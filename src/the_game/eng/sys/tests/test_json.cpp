#include "test_common.h"

#include <sstream>

#include "../Json.h"

namespace {
    const char json_example[] = "{"
            "\t\"widget\": {\n"
            "\t\t\"debug\": \"on\",\n"
            "\t\t\"window\": {\n"
            "\t\t\t\"title\": \"Sample Konfabulator Widget\",\n"
            "\t\t\t\"name\": \"main_window\",\n"
            "\t\t\t\"width\": 500,\n"
            "\t\t\t\"height\": 500\n"
            "\t\t},\n"
            "\t\t\"image\": { \n"
            "\t\t\t\"src\": \"Images/Sun.png\",\n"
            "\t\t\t\"name\": \"sun1\",\n"
            "\t\t\t\"hOffset\": -250,\n"
            "\t\t\t\"vOffset\": 250,\n"
            "\t\t\t\"alignment\": \"center\"\n"
            "\t\t},\n"
            "\t\t\"text\": {\n"
            "\t\t\t\"data\": \"Click Here\",\n"
            "\t\t\t\"size\": 36,\n"
            "\t\t\t\"style\": \"bold\",\n"
            "\t\t\t\"name\": \"text1\",\n"
            "\t\t\t\"hOffset\": 250,\n"
            "\t\t\t\"vOffset\": 100,\n"
            "\t\t\t\"alignment\": \"center\",\n"
            "\t\t\t\"onMouseUp\": \"sun1.opacity = (sun1.opacity / 100) * 90;\"\n"
            "\t\t}\n"
            "\t}"
            "}";

    const char json_example2[] = "{\n"
            "\t\"glossary\": {\n"
            "\t\t\"title\": \"example glossary\",\n"
            "\t\t\"GlossDiv\": {\n"
            "\t\t\t\"title\": \"S\",\n"
            "\t\t\t\"GlossList\": {\n"
            "\t\t\t\t\"GlossEntry\": {\n"
            "\t\t\t\t\t\"ID\": \"SGML\",\n"
            "\t\t\t\t\t\"SortAs\": \"SGML\",\n"
            "\t\t\t\t\t\"GlossTerm\": \"Standard Generalized Markup Language\",\n"
            "\t\t\t\t\t\"Acronym\": \"SGML\",\n"
            "\t\t\t\t\t\"Abbrev\": \"ISO 8879:1986\",\n"
            "\t\t\t\t\t\"GlossDef\": {\n"
            "\t\t\t\t\t\t\"para\": \"A meta-markup language, used to create markup languages such as DocBook.\",\n"
            "\t\t\t\t\t\t\"GlossSeeAlso\": [\"GML\", \"XML\"]\n"
            "\t\t\t\t\t},\n"
            "\t\t\t\t\t\"GlossSee\": \"markup\"\n"
            "\t\t\t\t}\n"
            "\t\t\t}\n"
            "\t\t}\n"
            "\t}\n"
            "}";

    const char json_example3[] = "{\n"
            "\t\"menu\": {\n"
            "\t\t\"id\": \"file\",\n"
            "\t\t\"value\": \"File\",\n"
            "\t\t\"popup\": {\n"
            "\t\t\t\"menuitem\": [\n"
            "\t\t\t\t{\"value\": \"New\", \"onclick\": \"CreateNewDoc()\"},\n"
            "\t\t\t\t{\"value\": \"Open\", \"onclick\": \"OpenDoc()\"},\n"
            "\t\t\t\t{\"value\": \"Close\", \"onclick\": \"CloseDoc()\"}\n"
            "\t\t\t]\n"
            "\t\t}\n"
            "\t}\n"
            "}";
}

void test_json() {

    {   // Test types
        {   // JsNumber
            JsNumber n1(3.1568);
            std::stringstream ss;
            n1.Write(ss);
            JsNumber n2;
            assert(n2.Read(ss));
            assert(n2 == Approx(n1));
            n1 = 5;
            n2 = 5;
            assert(n2 == double(5));
            assert(n1 == n2);
        }

        {   // JsString
            JsString s1("qwe123");
            std::stringstream ss;
            s1.Write(ss);
            JsString s2;
            assert(s2.Read(ss));
            assert(s1 == s2);
            s2 = "asd111";
            assert(s2 == "asd111");
        }

        {   // JsArray
            JsArray a1;
            a1.Push(1);
            a1.Push(2);
            a1.Push("qwe123");
            assert(a1.Size() == 3);
            assert((JsNumber)a1[0] == Approx(1));
            assert((JsNumber)a1[1] == Approx(2));
            assert((JsString)a1[2] == "qwe123");
            std::stringstream ss;
            a1.Write(ss);
            JsArray a2;
            assert(a2.Read(ss));
            assert(a2.Size() == 3);
            assert((JsNumber)a2[0] == Approx(1));
            assert((JsNumber)a2[1] == Approx(2));
            assert((JsString)a2[2] == "qwe123");

            assert_throws(a2.at(3));

            // check equality
            JsArray a3, a4, a5;
            a3.Push("asdf");
            a3.Push("zxc");
            a4.Push("asdf");
            a4.Push("zxc");
            a5.Push("asdf");
            a5.Push("zxc1");
            assert(a3 == a4);
            assert(a3 != a5);
        }

        {   // JsObject
            JsObject obj;
            obj["123"] = 143;
            obj["asdf"] = "asdfsdf";
            obj["123"] = 46;
            assert(obj.Size() == 2);
            assert((JsNumber)obj["123"] == 46);
            assert((JsString)obj["asdf"] == "asdfsdf");
            std::stringstream ss;
            obj.Write(ss);
            JsObject _obj;
            assert(_obj.Read(ss));
            assert(_obj.Size() == 2);
            assert((JsNumber)_obj["123"] == Approx(46));
            assert((JsString)_obj["asdf"] == "asdfsdf");

            assert_throws(_obj.at("non exists"));

            // check equality
            JsObject obj1, obj2, obj3;
            obj1["123"] = 143;
            obj1["asdf"] = "asdfsdf";
            obj2["123"] = 143;
            obj2["asdf"] = "asdfsdf";
            obj3["123"] = 143;
            obj3["asdf"] = "asdfsdf1";
            assert(obj1 == obj2);
            assert(obj1 != obj3);
        }

        {   // JsLiteral
            JsLiteral lit(JS_TRUE);
            assert(lit.val == JS_TRUE);
            std::stringstream ss;
            lit.Write(ss);
            JsLiteral _lit(JS_NULL);
            assert(_lit.Read(ss));
            assert(_lit.val == JS_TRUE);

            // check equality
            JsLiteral lit1(JS_FALSE), lit2(JS_FALSE), lit3(JS_NULL);
            assert(lit1 == lit2);
            assert(lit1 != lit3);
        }

        {   // JsElement
            JsElement _el1(16);
            const JsElement &el1 = _el1;
            assert_nothrow((JsNumber)_el1);
            assert_nothrow((const JsNumber &)el1);
            assert_throws((JsString)_el1);
            assert_throws((const JsString &)el1);
            assert_throws((JsArray)_el1);
            assert_throws((const JsArray &)el1);
            assert_throws((JsObject)_el1);
            assert_throws((const JsObject &)el1);
            assert_throws((JsLiteral)_el1);
            assert_throws((const JsLiteral &)el1);

            JsElement _el2("my string");
            const JsElement &el2 = _el2;
            assert_nothrow((JsString)_el2);
            assert_nothrow((const JsString &)el2);
            assert_throws((JsNumber)_el2);
            assert_throws((const JsNumber &)el2);
            assert_throws((JsArray)el2);
            assert_throws((const JsArray &)el2);
            assert_throws((JsObject)el2);
            assert_throws((const JsObject &)el2);
            assert_throws((JsLiteral)el2);
            assert_throws((const JsLiteral &)el2);

            JsElement _el3(JS_ARRAY);
            const JsElement &el3 = _el3;
            assert_nothrow((JsArray)_el3);
            assert_nothrow((const JsArray &)el3);
            assert_throws((JsNumber)_el3);
            assert_throws((const JsNumber &)el3);
            assert_throws((JsString)_el3);
            assert_throws((const JsString &)el3);
            assert_throws((JsObject)_el3);
            assert_throws((const JsObject &)el3);
            assert_throws((JsLiteral)_el3);
            assert_throws((const JsLiteral &)el3);

            JsElement _el4(JS_OBJECT);
            const JsElement &el4 = _el4;
            assert_nothrow((JsObject)_el4);
            assert_nothrow((const JsObject &)el4);
            assert_throws((JsNumber)_el4);
            assert_throws((const JsNumber &)el4);
            assert_throws((JsString)_el4);
            assert_throws((const JsString &)el4);
            assert_throws((JsArray)_el4);
            assert_throws((const JsArray &)el4);
            assert_throws((JsLiteral)_el4);
            assert_throws((const JsLiteral &)el4);

            JsElement _el5(JS_NULL);
            const JsElement &el5 = _el5;
            assert_nothrow((JsLiteral)_el5);
            assert_nothrow((const JsLiteral &)el5);
            assert_throws((JsNumber)_el5);
            assert_throws((const JsNumber &)el5);
            assert_throws((JsString)_el5);
            assert_throws((const JsString &)el5);
            assert_throws((JsArray)_el5);
            assert_throws((const JsArray &)el5);
            assert_throws((JsObject)_el5);
            assert_throws((const JsObject &)el5);
        }
    }

    {   // Complex test1
        bool flag = true;
        JsElement el(JS_NULL);
        std::stringstream ss(json_example);
        assert(el.Read(ss));

    AGAIN1:
        JsObject &root = (JsObject &)el;
        assert(root.Size() == 1);
        JsObject &widget = (JsObject &)root["widget"];
        assert(widget.Size() == 4);
        assert(widget["debug"] == "on");
        JsObject &window = (JsObject &)widget["window"];
        assert(window.Size() == 4);
        assert(window["title"] == "Sample Konfabulator Widget");
        assert(window["name"] == "main_window");
        assert(window["width"] == 500);
        assert(window["height"] == 500);
        JsObject &image = (JsObject &)widget["image"];
        assert(image.Size() == 5);
        assert(image["src"] == "Images/Sun.png");
        assert(image["name"] == "sun1");
        assert(image["hOffset"] == -250);
        assert(image["vOffset"] == 250);
        assert(image["alignment"] == "center");
        JsObject &text = (JsObject &)widget["text"];
        assert(text.Size() == 8);
        assert(text["data"] == "Click Here");
        assert(text["size"] == 36);
        assert(text["style"] == "bold");
        assert(text["name"] == "text1");
        assert(text["hOffset"] == 250);
        assert(text["vOffset"] == 100);
        assert(text["alignment"] == "center");
        assert(text["onMouseUp"] == "sun1.opacity = (sun1.opacity / 100) * 90;");

        if (flag) {
            flag = false;
            ss.clear();
            ss.seekg(0);
            root.Write(ss);
            goto AGAIN1;
        }
    }

    {   // Complex test2
        bool flag = true;
        JsElement el(JS_NULL);
        std::stringstream ss(json_example2);
        assert(el.Read(ss));

    AGAIN2:
        JsObject &root = (JsObject &)el;
        assert(root.Size() == 1);
        JsObject &glossary = (JsObject &)root["glossary"];
        assert(glossary.Size() == 2);
        assert(glossary["title"] == "example glossary");
        JsObject &gloss_div = (JsObject &)glossary["GlossDiv"];
        assert(gloss_div.Size() == 2);
        assert(gloss_div["title"] == "S");
        JsObject &gloss_list = (JsObject &)gloss_div["GlossList"];
        assert(gloss_list.Size() == 1);
        JsObject &gloss_entry = (JsObject &)gloss_list["GlossEntry"];
        assert(gloss_entry.Size() == 7);
        assert(gloss_entry["ID"] == "SGML");
        assert(gloss_entry["SortAs"] == "SGML");
        assert(gloss_entry["GlossTerm"] == "Standard Generalized Markup Language");
        assert(gloss_entry["Acronym"] == "SGML");
        assert(gloss_entry["Abbrev"] == "ISO 8879:1986");
        assert(gloss_entry["GlossSee"] == "markup");
        JsObject &gloss_def = (JsObject &)gloss_entry["GlossDef"];
        assert(gloss_def.Size() == 2);
        assert(gloss_def["para"] == "A meta-markup language, used to create markup languages such as DocBook.");
        JsArray &gloss_see_also = (JsArray &)gloss_def["GlossSeeAlso"];
        assert(gloss_see_also.Size() == 2);
        assert(gloss_see_also[0] == "GML");
        assert(gloss_see_also[1] == "XML");

        if (flag) {
            flag = false;
            ss.clear();
            ss.seekg(0);
            root.Write(ss);
            goto AGAIN2;
        }
    }

    {   // Complex test3
        bool flag = true;
        JsElement el(JS_NULL);
        std::stringstream ss(json_example3);

    AGAIN3:
        assert(el.Read(ss));

        JsObject &root = (JsObject &)el;
        assert(root.Size() == 1);
        JsObject &menu = (JsObject &)root["menu"];
        assert(menu.Size() == 3);
        assert(menu["id"] == "file");
        assert(menu["value"] == "File");
        JsObject &popup = (JsObject &)menu["popup"];
        assert(popup.Size() == 1);
        JsArray &menuitem = (JsArray &)popup["menuitem"];
        assert(menuitem.Size() == 3);
        JsObject &_0 = (JsObject &)menuitem[0];
        assert(_0.Size() == 2);
        JsObject &_1 = (JsObject &)menuitem[1];
        assert(_1.Size() == 2);
        JsObject &_2 = (JsObject &)menuitem[2];
        assert(_2.Size() == 2);
        assert(_0["value"] == "New");
        assert(_0["onclick"] == "CreateNewDoc()");
        assert(_1["value"] == "Open");
        assert(_1["onclick"] == "OpenDoc()");
        assert(_2["value"] == "Close");
        assert(_2["onclick"] == "CloseDoc()");

        if (flag) {
            flag = false;
            ss.clear();
            ss.seekg(0);
            root.Write(ss);
            std::string str = ss.str();
            goto AGAIN3;
        }
    }

    {   // Initializer lists
        JsArray arr = {0.0, 0.0, 1.0, 2.0, "str", JsArray{"qwe", 4.0}};
        assert(arr.at(0) == 0.0);
        assert(arr.at(1) == 0.0);
        assert(arr.at(2) == 1.0);
        assert(arr.at(3) == 2.0);
        assert(arr.at(4) == "str");
        assert(((const JsArray &)arr.at(5)).at(0) == "qwe");
        assert(((const JsArray &)arr.at(5)).at(1) == 4.0);
    }
}
