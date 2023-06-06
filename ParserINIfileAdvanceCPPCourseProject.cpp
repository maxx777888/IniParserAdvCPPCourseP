#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <windows.h>

class ini_parcer {

public:
    ini_parcer() = delete;
    //Constructor
    ini_parcer(std::string file_) : fileName(file_)
    {
        file_path.open(fileName);
        isFileSintaxCorrect(fileName);

        std::vector<std::string> v_str_file;
        std::map<std::string, std::vector<std::string>> m_map;
        std::string section_name = "";

        while (!file_path.eof())
        {
            std::string str;
            std::getline(file_path, str);
            if (str.find(';') != std::string::npos)
            {
                str.erase(str.find(';'));
            }

            if (str.find('[') != std::string::npos && str.find(']') != std::string::npos)
            {
                std::erase(str, '[');
                std::erase(str, ']');
                section_name = str;
                auto it = std::remove_if(section_name.begin(), section_name.end(), [](char const& c)
                    {
                        return std::isdigit(c);
                    });
                section_name.erase(it, section_name.end());
            }
            if (isFreeSpace(str))
            {
                continue;
            }
            v_str_file.push_back(str);
        }
        //Получаем вектор со всеми значениями, нужно убрать дубликаты и обновить значения
        for (int i = 0; i < v_str_file.size(); i++)
        {
            std::string p_str1 = "";
            if (v_str_file[i].find(section_name) != std::string::npos)
            {
                std::vector<std::string> vec_tech;
                int j = i + 1;
                p_str1 = v_str_file[i];
                if (j >= v_str_file.size())
                {
                    break;
                }
                for (j = i + 1; j < v_str_file.size(); j++)
                {
                    if (v_str_file[j].find(section_name) != std::string::npos)
                    {
                        auto res = m_map.emplace(p_str1, vec_tech);
                        if (!res.second)
                        {
                            auto it = m_map.find(p_str1);
                            if (it != m_map.end())
                            {
                                //Добавляем новые элементы в вектор, при условии что ключ совпал и уже есть
                                it->second.insert(it->second.begin(), vec_tech.begin(), vec_tech.end());
                                rewriting_vector_variables(it->second);

                            }
                        }
                        i = j - 1;
                        break;
                    }
                    vec_tech.push_back(v_str_file[j]);

                    if (j == v_str_file.size() - 1)
                    {
                        auto res = m_map.emplace(p_str1, vec_tech);
                        if (!res.second)
                        {
                            auto it = m_map.find(p_str1);
                            if (it != m_map.end())
                            {
                                it->second.insert(it->second.begin(), vec_tech.begin(), vec_tech.end());
                                rewriting_vector_variables(it->second);

                            }
                        }
                        i = j - 1;
                        break;
                    }
                }
            }
        }
        //Переписываем вектор на std::map 
        for (auto& it : m_map)
        {
            std::map<std::string, std::string> h_map;
            if (it.second.size() != 0)
            {
                for (int i = 0; i < it.second.size(); i++)
                {
                    if (it.second[i].find("=") != std::string::npos)
                    {
                        std::string var_name = it.second[i].substr(0, it.second[i].find("="));
                        //Проверка что если у названия переменной нет значений, пока передаю пустую строку
                        if (it.second[i].substr(it.second[i].find("=") + 1) == "")
                        {
                            h_map.emplace(var_name, "");
                        }
                        else
                        {
                            std::string var_value = it.second[i].substr(it.second[i].find("=") + 1, it.second[i].length());
                            h_map.emplace(var_name, var_value);
                        }

                    }
                    else
                    {
                        throw std::runtime_error("The syntax of the file is broken the variable has no equal sign!");
                    }

                }
                m_map_box.emplace(it.first, h_map);
            }
            else
            {
                m_map_box.emplace(it.first, h_map);
            }

        }
    }

    //Destructor
    ~ini_parcer()
    {
        if (file_path.is_open())
        {
            file_path.close();
        }
    }


    //Функция выводит данные файла на консоль
    void getFileData()
    {
        for (const auto& i : this->m_map_box) {
            std::cout << i.first << " ";
            for (auto& j : i.second) {
                std::cout << j.first << " => " << j.second << " ";
            }
            std::cout << std::endl;
        }
    }



    //Метод возвращает данные из файла
    template<class T>
    T get_value(std::string sec) {
        std::string section = sec.substr(0, sec.find("."));
        std::string var = sec.substr(sec.find(".") + 1, sec.length());

        if (checkTheCorrectSpelling(section, var))
        {
            auto result = m_map_box[section][var];
            if (result == "")
            {
                result = "The variable " + var + " has no value in the file";
            }
            return result;
        }
        return "NOT FOUND!!!";
    }
    //Спецификация предыдущего метода, когда запрос был на переменные типа int
    template<>
    int get_value <int>(std::string sec) {
        std::string section = sec.substr(0, sec.find("."));
        std::string var = sec.substr(sec.find(".") + 1, sec.length());

        if (checkTheCorrectSpelling(section, var))
        {
            auto result = m_map_box[section][var];
            if (result.find_first_not_of("0123456789") == std::string::npos)
            {
                int int_num = std::stoi(result);
                return int_num;
            }
            else {
                throw std::runtime_error("The type of variable isn't int!");
            }

        }

        return 0;
    }

private:

    std::ifstream file_path;
    std::string fileName;
    std::map<std::string, std::map<std::string, std::string>> m_map_box;

    //Метод проводит проверку правильного синтаксиса и выкидывает ошибку с номером строки, где произошла ошибка
    bool isFileSintaxCorrect(std::string file_adress)
    {
        std::ifstream file;
        file.open(file_adress);
        if (file.is_open())
        {
            int s_count = 0;
            while (!file.eof())
            {
                s_count++;
                std::string str;
                std::getline(file, str);
                if (str.find(';') == 0)
                {
                    continue;
                }
                if (str.find(';') != 0 && str.find(';', 0) != std::string::npos)
                {
                    if (isFreeSpace(str.substr(0, str.find(';'))))
                    {
                        continue;
                    }
                }
                if (isFreeSpace(str)) {
                    continue;
                }
                if (str.find('[', 0) != std::string::npos && str.find(']') == std::string::npos)
                {
                    std::string error = "Missing closing ] in the line " + std::to_string(s_count);
                    throw std::runtime_error(error);
                }
                if (str.find('[') == std::string::npos && str.find(']') != std::string::npos)
                {
                    std::string error = "Missing opening [ in the line " + std::to_string(s_count);
                    throw std::runtime_error(error);
                }
                if (str.find('[') == std::string::npos && str.find(']') == std::string::npos)
                {
                    if (str.find('=') == std::string::npos)
                    {
                        std::string error = "Missing equal = in the line " + std::to_string(s_count);
                        throw std::runtime_error(error);
                    }
                }

            }

            file.close();
            return true;
        }
        else {
            throw std::runtime_error("The file isn't valid or not found!");
        }
    }

    //Функция возвращает True, если строка без символов и пустая
    bool isFreeSpace(std::string s) {
        for (int i = 0; i < s.length(); i++) {
            if (!std::isspace(s[i]))
                return false;
        }
        return true;
    }

    //Проводим проверку на правильное заполнение запроса
    bool checkTheCorrectSpelling(std::string section, std::string var)
    {
        auto isSectionFound = m_map_box.find(section);
        bool h = false;
        if (isSectionFound == m_map_box.end())
        {
            hintOnRightSection();
            throw std::runtime_error("There is no section with this name in the file");
        }
        auto isVarFound = m_map_box[section].find(var);
        if (isVarFound == m_map_box[section].end())
        {
            hintOnRightVariable(section);
            throw std::runtime_error("There is no value for the specified variable in the file!");
        }
        if (isSectionFound != m_map_box.end() && isVarFound != m_map_box[section].end())
        {
            h = true;
        }
        return h;
    }
    //Метод проводит проверку секций
    void hintOnRightSection()
    {
        std::cout << "You may have made a misspelling - Sections in the file:" << std::endl;
        for (const auto& i : this->m_map_box)
        {
            std::cout << i.first << " ";
        }
    }
    //Метод проводит проверку переменных
    void hintOnRightVariable(std::string s_hint)
    {
        if (this->m_map_box[s_hint].size() == 0)
        {
            std::cout << "There are no variables in " << s_hint << "!" << std::endl;
        }
        else
        {
            std::cout << "You may have made a misspelling - variables in the " << s_hint << ":" << std::endl;
            for (const auto& i : this->m_map_box[s_hint])
            {
                std::cout << i.first << " ";
            }
        }

    }
    //Функция заменяет переменные, которые были переписаны
    void rewriting_vector_variables(std::vector<std::string>& v)
    {

        for (int i = 0; i < v.size() - 1; i++)
        {
            if (v[i].find("=") != std::string::npos)
            {
                std::string s1 = v[i].substr(0, v[i].find("="));
                for (int j = i + 1; j < v.size(); j++)
                {
                    if (v[j].find("=") != std::string::npos)
                    {
                        std::string s2 = v[j].substr(0, v[j].find("="));
                        if (s1 == s2)
                        {
                            v.erase(v.begin() + j);
                        }
                    }


                }
            }
        }
    }

};



int main()
{
    //Необходимые настройки для работы с русским языком
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");

    try
    {
        ini_parcer parser("ini_file.txt");
        //parser.getFileData();

        auto value = parser.get_value<int>("Section2.var1");
        auto value1 = parser.get_value<std::string>("Section4.Mode");

        std::cout << "The value is equal:  " << value << std::endl;
        std::cout << "The value1 is equal: " << value1 << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << " Exception: " << e.what() << std::endl;
    }

}
