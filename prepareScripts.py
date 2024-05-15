import os
import re


def prepareScripts():
    with open("src/scripts.cpp", "w") as f:
        f.write("#include <sstream>\n")
        files = os.listdir("scripts")
        for (n, file) in enumerate(files):
            with open("scripts/" + file, "r") as script:
                txt = script.read()

                inClass = False
                brace_level = 0
                classBody = []
                serializedFields = []
                className = ""

                lines = txt.split("\n")
                for line in lines:
                    stripped = line.strip()
                    if not inClass:
                        if stripped.startswith("class") and "public Script" in stripped:
                            inClass = True
                            brace_level = 0
                            classBody = [line]
                            serializedFields = []
                            rslt = re.search(
                                r"(?<=class)\s*(\w*)", stripped)
                            if rslt:
                                className = rslt.group(1)
                            continue
                        else:
                            f.write(line + "\n")
                            continue

                    classBody.append(line)

                    if "{" in stripped:
                        brace_level += 1
                    if "}" in stripped:
                        brace_level -= 1
                        if brace_level == 0:
                            inClass = False

                            serialize_regex = r'\[\[Serialize\]\]\s+((?:\w|:)+)\s+(\w+)'
                            matches = re.findall(
                                serialize_regex, "\n".join(classBody))
                            for match in matches:
                                serializedFields.append(
                                    (match[0], match[1]))

                            for i, line in enumerate(classBody):
                                classBody[i] = line.replace(
                                    "[[Serialize]]", "")

                            classBody[
                                -1] = "virtual bool Serialize(const std::string& __name, const std::string& __value) override {\n"
                            for field in serializedFields:
                                classBody.append(
                                    f"REGISTER_PROPERTY({field[0]}, {field[1]});\n")
                            classBody.append("return false;\n")
                            classBody.append("}\n")
                            classBody.append("};\n")
                            f.write("\n".join(classBody))

                            f.write(f"REGISTER_SCRIPT({className});\n")

                            continue


if __name__ == "__main__":
    prepareScripts()
