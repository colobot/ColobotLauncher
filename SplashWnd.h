/*! \file SplashWnd.h
\brief Класс, реализующий splash-окно в отдельном потоке. Без MFC.

Created 2006-06-08 by Kirill V. Lyadvinsky \n

* The contents of this file are subject to the terms of the Common Development and    \n
* Distribution License ("CDDL")(collectively, the "License"). You may not use this    \n
* file except in compliance with the License. You can obtain a copy of the CDDL at    \n
* http://www.opensource.org/licenses/cddl1.php.                                       \n

*/
#ifndef __SPLASHWND_H_
#define __SPLASHWND_H_

//! Класс, реализующий splash-окно в отдельном потоке.
class CSplashWnd
{
private:
	CSplashWnd(const CSplashWnd&) {};
	CSplashWnd& operator=(const CSplashWnd&) {};
protected:
	HANDLE							m_hThread;
	unsigned int				m_ThreadId;
	HANDLE							m_hEvent;

	Gdiplus::Image*			m_pImage;					//!< Выводимое изображение 
	HWND								m_hSplashWnd;			//!< Окно. Создается в отдельном потоке.
	std::string				m_WindowName;			//!< Заголовок окна. Выводится в task bar. Если пустой, то в task bar не будет кнопки.
	HWND								m_hProgressWnd;		//!< Progress bar. Выводиться внизу окна. Подробнее см. #SetProgress.
	HWND								m_hParentWnd;
	std::string				m_ProgressMsg;		//!< Кэш сообщения о ходе процесса.
  UINT_PTR            m_TimerId;        //!< Идентификатор созданного таймера.

public:
	CSplashWnd( HWND hParent = NULL );
	~CSplashWnd();														//!< Вызывает #Hide.

	void SetImage(Gdiplus::Image* pImage);		//!< Задать изображение явно. pImage можно удалять сразу после вызова этой ф-ии.
  void SetImage(HMODULE hModule, UINT nResourceID);	//!< Задать изображение через идентификатор ресурса. Загрузка идет из секции "IMAGE".
	void SetWindowName(LPCSTR windowName);		//!< Задать заголовок окна. Подробнее см. #m_WindowName.
	void Show();															//!< Показать окно. Это функция вызывается после всех Set*
	void Hide();															//!< Закрывает окно и останавливает поток.
  /*! Задает процент заполнения progress bar. Контрол создается при первом вызове ф-ии.
  @param[in] procent Процент выполнения
  */
  void SetProgress(UINT procent);
	/*! Задает процент заполнения progress bar. Контрол создается при первом вызове ф-ии.
		@param[in] procent Процент выполнения
		@param[in] msg Сообщение о процессе выполнения. Если указатель нулевой, то ничего не выводится.
	*/
	void SetProgress(UINT procent, const char* msg );
	/*! Задает процент заполнения progress bar. Контрол создается при первом вызове ф-ии.
	@param[in] procent Процент выполнения
	@param[in] nResourceID Ресурс, содержащий сообщение о процессе выполнения. Если нуль, то ничего не выводится.
	@param[in] hModule Модуль содержащий ресурсы. Если NULL, то загружается из исполняемого модуля.
	*/
	void SetProgress(UINT procent, UINT nResourceID = 0, HMODULE hModule = NULL );

	/*!\brief Автоматически меняет состояние progress bar. Останавливается досрочно при вызове #SetProgress.
			@param[in] from Начальная позиция
			@param[in] to Конечная позици
			@param[in] seconds Количество шагов (делается один шаг в секунду) */
	void SetAutoProgress(UINT from, UINT to, UINT steps);
	void SetProgressBarColor(COLORREF color);	//!< Задает цвет индикатора прогресса.

	HWND GetWindowHwnd() const								//!< Возвращает хендл окна
	{
		return m_hSplashWnd;
	};

protected:
	static LRESULT CALLBACK SplashWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static unsigned int __stdcall SplashThreadProc(void* lpParameter);
};

#endif//__SPLASHWND_H_
