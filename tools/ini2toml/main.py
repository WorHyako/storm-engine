import configparser
import tomlkit
import enum
import os


class ParseResult(enum.Enum):
    FLOAT = 1
    INT = 2
    STRING = 3


def element_type(s):
    try:
        int(s)
        return ParseResult.INT
    except ValueError:
        try:
            float(s)
            return ParseResult.FLOAT
        except ValueError:
            ...
    return ParseResult.STRING


def save_file(tomldoc, file_name):
    filename, file_extension = os.path.splitext(file_name)
    with open(f'{filename}.toml', 'w') as file:
        tomlkit.dump(tomldoc, file)
    print(f'{filename} was saved.')


for root, dirs, files in os.walk(os.getcwd(), topdown=False):
    for name in files:
        filename, file_extension = os.path.splitext(name)
        if file_extension != ".ini":
            continue
        # print(f'Work with - {os.path.join(root, name)}')
        file_full_path = os.path.join(root, name)
        file_relative_path = os.path.relpath(file_full_path, start=os.curdir)
        print(f'Work with - {file_relative_path}')

        config = configparser.ConfigParser()
        try:
            config.read(file_relative_path, 'utf-8-sig')
        except configparser.MissingSectionHeaderError:
            print("Fixing file header.")
            with open(file_relative_path, 'r+') as fd:
                contents = fd.readlines()
                contents.insert(0, "\n[Main]\n")
                fd.seek(0)
                fd.writelines(contents)
            config.read(file_relative_path, 'utf-8-sig')
        except configparser.DuplicateOptionError:
            with open(file_relative_path, 'r', -1, 'utf8') as file:
                content = file.readlines()

            print("content _____ ", content)
            i = 0
            while i < len(content) - 1:
                if content[i][0] == ';':
                    content.pop(i)
                equal_idx = content[i].find("=")
                if equal_idx != -1:
                    key = str(content[i][:equal_idx])
                    next_line_key_idx = content[i + 1].find(key)
                    if next_line_key_idx != -1:
                        next_line_equal_idx = content[i + 1].find("=")
                        content[i] = content[i].replace('\n', '')
                        content[i] += content[i + 1][next_line_equal_idx + 1:]
                        content.pop(i + 1)
                        continue
                i += 1
            print("new_content", content)
            with open(file_relative_path, 'w', -1, 'utf8') as file:
                file.writelines(content)

            config.read(file_relative_path, 'utf-8-sig')

        toml_doc = tomlkit.document()

        print(config.sections())
        for section in config.sections():
            toml_section = tomlkit.table()
            for key, value in config[section].items():
                value = value.replace(',', ' ')
                value = value.split(' ')
                value = list(filter(len, value))
                match element_type(value[0]):
                    case ParseResult.STRING:
                        res = value[0] if len(value) == 1 else value
                    case ParseResult.INT:
                        res = int(value[0]) if len(value) == 1 else list(map(int, value))
                    case ParseResult.FLOAT:
                        res = float(value[0]) if len(value) == 1 else list(map(float, value))
                toml_section.add(key, res)
            toml_doc.add(section, toml_section)
        save_file(toml_doc, file_relative_path)
