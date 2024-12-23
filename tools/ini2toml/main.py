import configparser
import sys
import tomlkit
import enum
import os


class ParseResult(enum.Enum):
    FLOAT = 1
    INT = 2
    STRING = 3


def split_with_symbol_saving(file_content) -> list:
    content = file_content.splitlines()
    content = [(line + '\n') for line in content]
    return content


def element_type(element) -> ParseResult:
    """
    Check type of variable. It needs for toml document formating

    :param  element:    Var to check tpe
    :type   element:    any

    :return:    Var type via ParseResult
    :rtype:     ParseResult
    """
    try:
        int(element)
        return ParseResult.INT
    except ValueError:
        try:
            float(element)
            return ParseResult.FLOAT
        except ValueError:
            ...
    return ParseResult.STRING


def list_type(elements) -> ParseResult:
    """
    Check type of list. It needs for toml document formating

    :param  elements:   Var to check tpe
    :type   elements:   any

    :return:    List type via ParseResult
    :rtype:     ParseResult
    """
    try:
        for element in elements:
            if type(element) is not list:
                int(element)
            else:
                for j in element:
                    int(j)
        return ParseResult.INT
    except ValueError:
        try:
            for element in elements:
                if type(element) is not list:
                    float(element)
                else:
                    for j in element:
                        float(j)
            return ParseResult.FLOAT
        except ValueError:
            ...
    return ParseResult.STRING


def save_file(tomldoc, file_path) -> None:
    """
    Save toml document's content to file

    :param  tomldoc:    toml document
    :type   tomldoc:    tomlkit.document

    :param  file_name:  File name
    :type   file_name:  str
    """
    # filename, file_extension = os.path.splitext(file_name)
    with open(f'{file_path[:-4]}.toml', 'w', encoding='utf-8') as file:
        tomlkit.dump(tomldoc, file)


def repair_option_duplicating(file_content) -> str:
    """

    1. Searches key in each section.

    2. If first key was found, checks all current section for key duplication.

    2.

    :param  file_content:  File path
    :type   file_content:  str
    """
    content = split_with_symbol_saving(file_content)
    i = 0
    while i < len(content) - 1:
        equal_idx = content[i].find("=")
        if equal_idx == -1:
            i += 1
            continue
        key = str(content[i][:equal_idx - 1])
        j = i + 1
        while j < len(content) and not content[j].startswith('['):
            if not content[j].startswith(f'{key} '):
                j += 1
                continue
            current_line = content[i][:-1]
            other_line_equal_idx = content[j].find('=')
            value_next = content[j][other_line_equal_idx + 2:-1]
            value_next_arr = value_next.split(',')

            current_line += ',' + ','.join(value_next_arr)
            content[i] = current_line + '\n'
            content.pop(j)
            continue
        i += 1
    return ''.join(content)


def repair_section_duplicating(file_content) -> str:
    content = split_with_symbol_saving(file_content)
    for i in range(len(content) - 1):
        if not content[i].startswith('[') and not content[i].endswith(']\n'):
            continue
        section_name = content[i][1:-2]
        for j in range(i + 1, len(content) - 1):
            if content[j].find(f'[{section_name}]') == -1:
                continue
            content[j] = f'[{section_name}_]\n'
            break
    return ''.join(content)


def repair_symbols(file_content) -> str:
    """
    Rewrites selected file with fixed next cases:

    1. Delete all comments

    2. Replace all '\t' symbols to spaces.

    3. Replace all multispaces places

    4. Replace all `%` symbols to `%%`, coz it's toml formate rule

    5. Remove all empty lines

    6. Fill all empty options with zero value. F.e. `option` becomes `option = 0`

    7. If option is array, puts it bracket ("[...]")

    :param  file_content:  File's content
    :type   file_content:  str
    """
    content = split_with_symbol_saving(file_content)
    i = 0
    while i < len(content):
        content[i] = content[i].replace('\t', ' ').replace('  ', ' ').replace('  ', ' ')
        if content[i].isspace():
            content.pop(i)
            continue

        comment_idx = content[i].find(';')
        if comment_idx != -1:
            if comment_idx < 3:
                content.pop(i)
                continue
            content[i] = content[i][:comment_idx]
            if content[i].endswith(' '):
                content[i] = content[i][:-1]
            if not content[i].endswith('\n'):
                content[i] += '\n'

        if content[i].startswith(' '):
            content[i] = content[i][1:]

        percent_symbol_idx = content[i].find('%')
        if percent_symbol_idx != -1:
            content[i] = content[i].replace('%', '%%')

        if len(content[i]) > 1 and not content[i].startswith('[') and content[i].find('=') == -1:
            content[i] = content[i][:-1] + ' = 0\n'

        if content[i] == '\n' or content[i].replace(' ', '') == '':
            content.pop(i)
            continue

        equal_idx = content[i].find('=')
        separ_idx = content[i].find(',')
        if separ_idx != -1:
            content[i] = content[i].replace(', ', ',')
            content[i] = content[i][:equal_idx + 2] + '[' + content[i][equal_idx + 2:-1] + ']\n'

        if equal_idx != -1:
            key = content[i][:equal_idx - 1]
            if key.find(' '):
                content[i] = key.replace(' ', '_') + content[i][equal_idx - 1:]

        open_bracket_idx = content[i].find('[')
        close_bracket_idx = content[i].find(']')
        if open_bracket_idx == 0 and close_bracket_idx != -1:
            section_name = content[i][open_bracket_idx + 1 : close_bracket_idx]
            if section_name.find(' ') != -1:
                section_name = section_name.replace(' ', '_')
                content[i] = '[' + section_name + ']\n'
        i += 1
    return ''.join(content)


def repair_header_missing(file_content) -> str:
    """
    Insert default header section `[Main]` in file.

    :param  file_content:  File's content
    :type   file_content:  str
    """
    return '\n[Main]\n' + file_content


def main() -> int:
    for root, dirs, files in os.walk(os.getcwd(), topdown=False):
        for name in files:
            file_full_path = os.path.join(root, name)
            filename, file_extension = os.path.splitext(name)
            if file_extension != ".ani" and file_extension != ".ini" or filename.endswith("_new"):
                continue

            if filename == 'fonts' or filename == 'fonts_rus':
                print('Skipping font file')
                continue

            with open(file_full_path, 'r', encoding='utf-8', errors='ignore') as file:
                file_content = file.read()
            print(f'Work with - {file_full_path}')

            file_content = repair_symbols(file_content)

            parser = configparser.ConfigParser()
            while True:
                try:
                    parser.read_string(file_content)
                    break

                except configparser.MissingSectionHeaderError:
                    print("Fixing file header.")
                    file_content = repair_header_missing(file_content)

                except configparser.DuplicateOptionError:
                    print("Fixing file option duplicating.")
                    file_content = repair_option_duplicating(file_content)

                except configparser.DuplicateSectionError:
                    print("Fixing file section duplicating.")
                    file_content = repair_section_duplicating(file_content)

            toml_doc = tomlkit.document()

            for section in parser.sections():
                toml_section = tomlkit.table()
                for key, value in parser[section].items():
                    value_arr = []
                    cur = prev = 0
                    while cur < len(value):
                        if value[cur] == ',':
                            value_arr.append(value[prev:cur])
                            prev = cur + 1
                        if value[cur] == '[':
                            array_value = []
                            array_cur = cur + 1
                            array_prev = array_cur
                            while value[array_cur] != ']':
                                if value[array_cur] == ',':
                                    array_value.append(value[array_prev:array_cur])
                                    array_prev = array_cur + 1
                                array_cur += 1
                            array_value.append(value[array_prev:array_cur])
                            value_arr.append(array_value)
                            cur = array_cur + 1
                            prev = cur
                        cur += 1
                    if cur - 1 != prev or len(value) == 1:
                        value_arr.append(value[prev:cur])

                    for i in range(len(value_arr)):
                        if type(value_arr[i]) is list:
                            value_arr[i] = [each.replace('"', '') for each in value_arr[i]]
                        else:
                            value_arr[i] = value_arr[i].replace('"', '')
                    res = []
                    match list_type(value_arr):
                        case ParseResult.STRING:
                            for i in value_arr:
                                if type(i) is list:
                                    res.append(list(map(str, i)))
                                    continue
                                res.append(str(i))
                        case ParseResult.INT:
                            for i in value_arr:
                                if type(i) is list:
                                    res.append(list(map(int, i)))
                                    continue
                                res.append(int(i))
                        case ParseResult.FLOAT:
                            for i in value_arr:
                                if type(i) is list:
                                    res.append(list(map(float, i)))
                                    continue
                                res.append(float(i))
                    if len(res) == 1:
                        res = res[0]
                    toml_section.add(key, res)
                toml_doc.add(section, toml_section)
            save_file(toml_doc, f'{file_full_path}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
