#ifndef __QUIZ_HPP__
#define __QUIZ_HPP__
#include <iostream>
#include <string>
#include <tchar.h>
#if defined(UNICODE)
#define OUTPUT std::wcout;
#else
#define OUTPUT std::cout
#endif

class Quiz {
private:
	std::basic_string<TCHAR> Question;
	std::basic_string<TCHAR> Explanation;
	int CorrectAnswer;
	
public:
	Quiz(const std::basic_string<TCHAR> Question, const std::basic_string<TCHAR> Explanation, const bool IsNotForBeginner, const int NumberOfChoices, const int CorrectAnswer)
		: Question(Question), Explanation(Explanation), CorrectAnswer(CorrectAnswer), NumberOfChoices(NumberOfChoices), IsNotForBeginner(IsNotForBeginner) {}
	void PrintQuestion() const { OUTPUT << this->Question << std::endl; }
	bool JudgeCorrect(const int UserAnswer) const { return this->CorrectAnswer == UserAnswer; }
	void PrintCorrectAnswer() const { OUTPUT << _T("³‰ð‚Í") << this->CorrectAnswer << _T("‚Å‚·B") << std::endl; }
	void PrintExplanation() const { OUTPUT << this->Explanation << std::endl; }
	int NumberOfChoices;
	bool IsNotForBeginner;
};
#endif
