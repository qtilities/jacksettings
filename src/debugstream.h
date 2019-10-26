#pragma once

#include <QObject>
#include <src/ui/debuglogger.h>
#include <QMutex>

class DebugStream : public QObject, std::basic_streambuf<char>
{
	Q_OBJECT

public:
	DebugStream(std::ostream &stream, DebugLogger *textEdit)
	:
	outStream(stream)
	{
		txtLogger = textEdit;
		oldBuffer = stream.rdbuf();
		stream.rdbuf(this);

		connect(textEdit, &DebugLogger::appendPlainText,
						this, &DebugStream::sendString);
	}
	~DebugStream()
	{
		// output anything that is left
		if (!textString.empty())
			emit sendString(textString.c_str());

		//txtLogger->appendPlainText(textString.c_str());
		outStream.rdbuf(oldBuffer);
	}

signals:
	void sendString(QString text);

protected:
	virtual int_type overflow(int_type v)
	{
		mutex.lock();
		if (v == '\n')
		{
			emit sendString(textString.c_str());
			//txtLogger->appendPlainText(textString.c_str());
			textString.erase(textString.begin(), textString.end());
		}
		else
			textString += v;

		mutex.unlock();
		return v;
	}
	virtual std::streamsize xsputn(const char *c, std::streamsize n)
	{
		mutex.lock();

		textString.append(c, c + n);
		size_t pos = 0;
		while (pos != std::string::npos)
		{
			pos = textString.find('\n');

			if (pos != std::string::npos)
			{
				std::string tmp(textString.begin(), textString.begin() + pos);
				emit sendString(tmp.c_str());
				//txtLogger->appendPlainText(tmp.c_str());
				textString.erase(textString.begin(), textString.begin() + pos + 1);
			}
		}
		mutex.unlock();
		return n;
	}

private:
	std::ostream   &outStream;
	std::streambuf *oldBuffer;
	std::string     textString;
	DebugLogger    *txtLogger;
	QMutex          mutex;
};
