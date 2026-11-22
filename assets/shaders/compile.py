import asyncio
from pathlib import Path
import time

# Config
SCRIPT_DIR = Path(__file__).resolve().parent
SRC_DIR = SCRIPT_DIR / "src"
BIN_DIR = SCRIPT_DIR / "bin"
COMPILER_PATH = "slangc"
SHADER_EXTENSIONS = {".slang"}
# ---


class ShaderCompiler:
    def __init__(self, src: Path, bin: Path):
        self.src_dir = src
        self.bin_dir = bin

    async def compile_all(self) -> None:
        start_time = time.time()

        if not self.src_dir.exists():
            print(f"Error: Source directory '{self.src_dir}' not found")
            return

        self.bin_dir.mkdir(parents=True, exist_ok=True)

        print(f"Compiling...")
        print(f"Source: {self.src_dir}")
        print(f"Binaries: {self.bin_dir}")
        print("-" * 50)

        tasks = []
        for shader_file in self.src_dir.rglob("*"):
            if shader_file.suffix in SHADER_EXTENSIONS:
                tasks.append(self._compile_file(shader_file))

        if not tasks:
            print("No shaders found.")
            return

        await asyncio.gather(*tasks)

        end_time = time.time()
        elapsed = (end_time - start_time) * 1000
        print("-" * 50)
        print(f"Total async compilation time: {elapsed:.0f} ms")

    async def _compile_file(self, shader_path: Path) -> None:
        relative_path = shader_path.relative_to(self.src_dir)
        output_path = self.bin_dir / f"{relative_path}.spv"
        output_path.parent.mkdir(parents=True, exist_ok=True)

        process = await asyncio.create_subprocess_exec(
            COMPILER_PATH,
            str(shader_path),
            "-o",
            str(output_path),
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
        )

        stdout, stderr = await process.communicate()

        if process.returncode == 0:
            print(f"[SUCCESS] {relative_path}")
        else:
            error_msg = stderr.decode().strip()
            print(f"[ERROR] {relative_path}:\n{error_msg}")


def main():
    compiler = ShaderCompiler(SRC_DIR, BIN_DIR)

    try:
        asyncio.run(compiler.compile_all())
    except FileNotFoundError:
        print(f"[ERROR] Compiler '{COMPILER_PATH}' not found in PATH")
    except KeyboardInterrupt:
        print("\nCompilation canceled")


if __name__ == "__main__":
    main()
