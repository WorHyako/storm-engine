import configparser
import tomlkit
import enum
import os
import shutil


class ParseResult(enum.Enum):
    FLOAT = 1
    INT = 2
    STRING = 3


def element_type(elements):
    try:
        for i in elements:
            int(i)
        return ParseResult.INT
    except ValueError:
        try:
            for i in elements:
                float(i)
            return ParseResult.FLOAT
        except ValueError:
            ...
    return ParseResult.STRING


def save_file(tomldoc, file_name):
    filename, file_extension = os.path.splitext(file_name)
    with open(f'{filename[:-4]}.toml', 'w', encoding='utf-8') as file:
        tomlkit.dump(tomldoc, file)


def repair_option_duplicating(file_path):
    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.readlines()

    i = 0
    while i < len(content) - 1:
        if content[i][0] == ';' or content[i] == "\n":
            content.pop(i)
            continue
        content[i] = content[i].replace("\t", " ")
        content[i] = content[i].replace("  ", " ")
        if content[i][0] != '[' and content[i].find('=') == -1:
            content[i] = content[i].replace("\n", "") + "= 0\n"
        i += 1

    i = 0
    while i < len(content) - 1:
        equal_idx = content[i].find("=")
        if equal_idx != -1:
            key = str(content[i][:equal_idx].replace(" ", ""))
            j = i + 1
            while j < len(content) and content[j].find("[") == -1:
                other_line_key_idx = content[j].find(key)
                if other_line_key_idx != -1:
                    content[i] = content[i].replace('\n', '')
                    other_line_equal_idx = content[j].find('=')
                    content[i] += content[j][other_line_equal_idx + 1:]
                    content.pop(j)
                    continue
                j += 1
        i += 1
    with open(file_path, 'w', encoding='utf-8') as file:
        file.writelines(content)


def repair_section_duplicating():
    ...


def repair_header_missing(file_path):
    print("Fixing file header.")
    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.readlines()

    content.insert(0, "\n[Main]\n")
    with open(file_path, 'w', encoding='utf-8') as file:
        file.writelines(content)


for root, dirs, files in os.walk(os.getcwd(), topdown=False):
    for name in files:
        file_full_path = os.path.join(root, name)
        filename, file_extension = os.path.splitext(name)
        if file_extension != ".ini" or filename.endswith("_new"):
            continue

        ini_file = f'{root}\\{filename}_new{file_extension}'
        print(f'Work with - {ini_file}')

        shutil.copy(file_full_path, ini_file)

        repaired = False
        parser = configparser.ConfigParser()
        while not repaired:
            try:
                parser.read(ini_file, 'utf-8')
                repaired = True
                break

            except configparser.MissingSectionHeaderError:
                print("Fixing file header.")
                repair_header_missing(ini_file)

            except configparser.DuplicateOptionError:
                print("Fixing file option duplicating.")
                repair_option_duplicating(ini_file)

        toml_doc = tomlkit.document()

        for section in parser.sections():
            toml_section = tomlkit.table()
            for key, value in parser[section].items():
                value = value.replace(',', ' ')
                value = value.split(' ')
                value = list(filter(len, value))
                match element_type(value):
                    case ParseResult.STRING:
                        res = value[0] if len(value) == 1 else value
                    case ParseResult.INT:
                        res = int(value[0]) if len(value) == 1 else list(map(int, value))
                    case ParseResult.FLOAT:
                        res = float(value[0]) if len(value) == 1 else list(map(float, value))
                toml_section.add(key, res)
            toml_doc.add(section, toml_section)
        save_file(toml_doc, ini_file)
