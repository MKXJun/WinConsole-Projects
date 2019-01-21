#pragma once
#include <Windows.h>
#include <cstdio>
#include <memory>




class Console
{
public:
	enum Color
	{
		Black = 0,
		DarkBlue = 1,
		DarkGreen = 2,
		DarkCyan = 3,
		DarkRed = 4,
		DarkMagenta = 5,
		DarkYellow = 6,
		DarkWhite = 7,
		Gray = 8,
		Blue = 9,
		Green = 10,
		Cyan = 11,
		Red = 12,
		Magenta = 13,
		Yellow = 14,
		White = 15
	};


#pragma region ����̨�������
	/// <summary>
	/// ����ַ���
	/// </summary>
	static int Write(const char * Str) noexcept;

	/// <summary>
	/// ����ַ�
	/// </summary>
	static int WriteChar(const char Ch) noexcept;

	/// <summary>
	/// ��ʽ�����
	/// </summary>
	static int WriteFormat(const char * const Format, ...) noexcept;

	/// <summary>
	/// ����ַ�������
	/// </summary>
	static int WriteLine(const char * Str) noexcept;

	/// <summary>
	/// ָ��������λ������ַ���������ı���λ��
	/// </summary>
	/// <param name = "X">������X����λ��(0��ʼ)</param>
	/// <param name = "Y">������Y����λ��(0��ʼ)</param>
	/// <return>�ɹ�������ַ���Ŀ</return>
	static int WriteBuffer(short X, short Y, const char * Str) noexcept;


#pragma endregion

#pragma region ����̨���뷽��
	/// <summary>
	/// ��ʽ������
	/// </summary>
	static int ReadFormat(const char * const Format, ...) noexcept;

	/// <summary>
	/// �����ַ���
	/// </summary>
	static char * ReadLine(char * const Buffer, size_t BufferCount) noexcept;

	/// <summary>
	/// �����ַ�
	/// </summary>
	static char ReadChar() noexcept;
#pragma endregion

#pragma region ����̨��ȡ����(�赥��ʵ����)
	/// <summary>
	/// ��ȡ����̨�ĵ���(��Ҫ��ʵ����)
	/// </summary>
	static Console& Get();

	/// <summary>
	/// ��ȡ����̨����������
	/// </summary>
	static BOOL GetBuffer(CHAR_INFO* pBufferOut);


	/// <summary>
	/// ��ȡ����̨�������Ŀ�Ⱥ͸߶�
	/// </summary>
	static COORD GetBufferSize();

	/// <summary>
	/// ��ȡ����̨����λ������
	/// </summary>
	static COORD GetCursorPostion();

	/// <summary>
	/// ��ȡ����̨���Ĵ�С
	/// </summary>
	static DWORD GetCursorSize();

	/// <summary>
	/// ��ȡ����̨���Ŀɼ���
	/// </summary>
	static BOOL GetCursorVisible();

	/// <summary>
	/// ��ȡ����̨�����¼
	/// </summary>
	static INPUT_RECORD GetInputRecord();

	/// <summary>
	/// ��ȡ����̨����Ĵ���ҳ
	/// </summary>
	static UINT GetInputCodePage();

	/// <summary>
	/// ��ȡ����̨����Ĵ���ҳ
	/// </summary>
	static UINT GetOutputCodePage();

	/// <summary>
	/// ��ȡ����̨������
	/// </summary>
	/// <param name = "Buffer">[Out]��������������Ļ�����</param>
	/// <param name = "BufferCount">[In]������Ԫ�ظ���</param>
	/// <return>���ش�����</return>
	static errno_t GetWindowName(char * Buffer, size_t BufferCount);



#pragma endregion

#pragma region ����̨�޸ķ���(�赥��ʵ����)
	/// <summary>
	/// ���ÿ���̨�������Ŀ�Ⱥ͸߶�
	/// </summary>
	/// <param name = "Width">���������</param>
	/// <param name = "Height">�������߶�</param>
	/// <return>�����Ƿ�ɹ�</return>
	static BOOL SetBufferSize(short Width, short Height);

	/// <summary>
	/// ���ù���ȺͿɼ���
	/// </summary>
	/// <param name = "Weight">����Ե����ϵĸ��ǳ̶�(1-100)</param>
	/// <param name = "Visible">���ɼ���</param>
	/// <return>�����Ƿ�ɹ�</return>
	static BOOL SetCursorInfo(unsigned Weight, bool Visible);

	/// <summary>
	/// ���ù��λ��
	/// </summary>
	/// <param name = "X">���ˮƽλ��(��0��ʼ)</param>
	/// <param name = "Y">�����ֱλ��(��0��ʼ</param>
	/// <return>�����Ƿ�ɹ�</return>
	static BOOL SetCursorPos(short X, short Y);

	/// <summary>
	/// ���ÿ���̨�������ҳ
	/// </summary>
	/// <param name = "CodePage">����̨�������ҳ</param>
	/// <return>�����Ƿ�ɹ�</return>
	static BOOL SetInputCodePage(unsigned CodePage);

	/// <summary>
	/// ���ÿ���̨�������ҳ
	/// </summary>
	/// <param name = "CodePage">����̨�������ҳ</param>
	/// <return>�����Ƿ�ɹ�</return>
	static BOOL SetOutputCodePage(unsigned CodePage);

	/// <summary>
	/// �����ı���ɫ�ͱ�����ɫ
	/// </summary>
	/// <param name = "foreGround">�ı���ɫ</param>
	/// <param name = "backGround">������ɫ</param>
	/// <return>�����Ƿ�ɹ�</return>
	static BOOL SetTextColor(Color foreGround = Color::DarkWhite, Color backGround = Color::Black);

	/// <summary>
	/// ���ÿ���̨����
	/// </summary>
	/// <param name = "Title">����̨����</param>
	/// <return>�����Ƿ�ɹ�</return>
	static BOOL SetTitle(const char * Title);

	/// <summary>
	/// ���ô��ڴ�С
	/// </summary>
	/// <param name = "Width">����̨���ڿ��</param>
	/// <param name = "Height">����̨���ڸ߶�</param>
	/// <return>�����Ƿ�ɹ�</return>
	static BOOL SetWindowSize(short Width, short Height);
#pragma endregion

	Console& operator=(const Console& CopyFrom) = delete;
	Console& operator=(Console&& MoveFrom) noexcept;
	Console(const Console& CopyFrom) = delete;
	Console(Console&& MoveFrom) noexcept;
	Console();
	~Console();

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};


