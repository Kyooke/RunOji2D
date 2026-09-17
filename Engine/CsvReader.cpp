#include <Windows.h>
#include "CsvReader.h"


//コンストラクタ
CsvReader::CsvReader()
{
	data_.clear();
}

//デストラクタ
CsvReader::~CsvReader()
{
	//全データを開放
	for (int y = 0; y < data_.size(); y++)
	{
		for (int x = 0; x < data_[y].size(); x++)
		{
			data_[y][x].clear();
		}
	}
}

//CSVファイルのロード
bool CsvReader::Load(std::string fileName)
{
	HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		std::string message = "「" + fileName + "」が開けません。\n開いている場合は閉じてください。";
		MessageBox(NULL, message.c_str(), "エラー", MB_OK);
		return false;
	}

	DWORD fileSize = GetFileSize(hFile, NULL);

	// ★ 安全に文字列として読み込むため、+1 サイズにして \0 を保証する
	char* temp = new char[fileSize + 1];
	DWORD dwBytes = 0;
	ReadFile(hFile, temp, fileSize, &dwBytes, NULL);
	temp[fileSize] = '\0'; // 終端文字を追加！
	CloseHandle(hFile);

	// stringとして保持する
	std::string fileContent(temp, fileSize);
	delete[] temp;

	data_.clear();
	std::vector<std::string> line;
	DWORD index = 0;

	while (index < fileSize)
	{
		std::string val;
		// fileContent を渡すようにする
		while (index < fileSize && fileContent[index] != ',' && fileContent[index] != '\n' && fileContent[index] != '\r')
		{
			val += fileContent[index];
			index++;
		}

		// 改行やカンマのスキップ
		if (index < fileSize) {
			char c = fileContent[index];
			index++;
			// \r\n などの連続改行に対応
			if (c == '\r' && index < fileSize && fileContent[index] == '\n') {
				index++;
			}
		}

		line.push_back(val);

		// 行末（改行）に達したときの判定
		// （次の文字がない、あるいは今のが改行だった場合など）
		if (index >= fileSize || fileContent[index - 1] == '\n' || fileContent[index - 1] == '\r')
		{
			data_.push_back(line);
			line.clear();
		}
	}

	return true;
}

//「,」か「改行」までの文字列を取得
void CsvReader::GetToComma(std::string* result, std::string data, DWORD* index)
{
	// ★ 文字列の範囲内であるかチェックする安全ガードを追加
	while (*index < data.length() && data[*index] != ',' && data[*index] != '\n' && data[*index] != '\r')
	{
		*result += data[*index];
		(*index)++;
	}

	if (*index < data.length()) {
		(*index)++; // カンマや改行をスキップ
	}

	*result += '\0';
}

//指定した位置のデータを文字列で取得
std::string CsvReader::GetString(DWORD x, DWORD y)
{
	if (x < 0 || x >= GetWidth() || y < 0 || y >= GetHeight())
		return "";

	return data_[y][x];
}

//指定した位置のデータを整数で取得
int CsvReader::GetValue(DWORD x, DWORD y)
{
	return atoi(GetString(x, y).c_str());
}

//ファイルの列数を取得
size_t CsvReader::GetWidth()
{
	return data_[0].size();
}

//ファイルの行数を取得
size_t CsvReader::GetHeight()
{
	return data_.size();
}