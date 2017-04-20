#ifndef ISERIALIZABLE_H
#define ISERIALIZABLE_H

struct JsObject;

class ISerializable {
public:
	virtual bool Read(const JsObject &js_in) = 0;
    virtual void Write(JsObject &js_out) = 0;
};

#endif // ISERIALIZABLE_H
