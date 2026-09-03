from pathlib import Path
import subprocess

SCRIPT_PATH = Path(__file__).parent

SLANGC_PATH = "slangc"
INPUT_SHADER = SCRIPT_PATH / "src/shader.slang"
OUTPUT_SPV = SCRIPT_PATH / "bin/shader.spv"


def main():
    args = [
        SLANGC_PATH,
        INPUT_SHADER,
        "-target",
        "spirv",
        "-profile",
        "spirv_1_3",
        "-emit-spirv-directly",
        "-fvk-use-entrypoint-name",
        "-entry",
        "vertMain",
        "-entry",
        "fragMain",
        "-o",
        OUTPUT_SPV,
    ]
    subprocess.run(args)


if __name__ == "__main__":
    main()
