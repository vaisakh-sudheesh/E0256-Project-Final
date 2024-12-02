/**
 * @file library_func_dump.cpp
 * @author Vaisakh P S <vaisakhp@iisc.ac.in>
 * @brief Extracts the names of all functions in a shared library and writes them to a file.
 *
 * @details This program extracts the names of all functions in a shared library and writes them to a file.
 *          The output file contains one function name per line, prefixed by the function's index in the symbol table.
 *          The function names are extracted from the symbol table of the shared library.
 *
 * @example ./library_func_dump <shared-library> <output-file>
 *
 * @note This program uses the ELF library to parse the shared library.
 *       Hence ensure that the ELF library is installed and linked to in the CMakeLists.txt file.
 */
#include <iostream>
#include <fstream>
#include <libelf.h>
#include <fcntl.h>
#include <gelf.h>
#include <unistd.h>

/**
 * @brief Extracts the names of all functions in the shared library at the given path and writes them to the output file.
 *
 */
void extractFunctionNames(const char *libPath, const char *outputFile) {
    // Initialize the ELF library and perform sanity checks
    if (elf_version(EV_CURRENT) == EV_NONE) {
        std::cerr << "ELF library initialization failed: " << elf_errmsg(-1) << std::endl;
        return;
    }

    // Open the shared library file
    int fd = open(libPath, O_RDONLY, 0);
    if (fd < 0) {
        std::cerr << "Failed to open file: " << libPath << std::endl;
        return;
    }

    // Initialize the ELF descriptor
    Elf *elf = elf_begin(fd, ELF_C_READ, nullptr);
    if (!elf) {
        std::cerr << "elf_begin() failed: " << elf_errmsg(-1) << std::endl;
        close(fd);
        return;
    }

    // Iterate over the sections to find the symbol table
    Elf_Scn *scn = nullptr;
    GElf_Shdr shdr;
    while ((scn = elf_nextscn(elf, scn)) != nullptr) {
        gelf_getshdr(scn, &shdr);
        if (shdr.sh_type == SHT_SYMTAB || shdr.sh_type == SHT_DYNSYM) {
            break;
        }
    }

    // Check if the symbol table was found, if not print an error message
    if (!scn) {
        std::cerr << "No symbol table found in " << libPath << std::endl;
        elf_end(elf);
        close(fd);
        return;
    }

    // Get the symbol table data, exit in case of error.
    Elf_Data *data = elf_getdata(scn, nullptr);
    if (!data) {
        std::cerr << "elf_getdata() failed: " << elf_errmsg(-1) << std::endl;
        elf_end(elf);
        close(fd);
        return;
    }

    // Open the output file for writing
    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open output file: " << outputFile << std::endl;
        elf_end(elf);
        close(fd);
        return;
    }

    // Iterate over the symbol table entries and extract function names
    int count = shdr.sh_size / shdr.sh_entsize;
    for (int i = 0; i < count; ++i) {
        GElf_Sym sym;
        gelf_getsym(data, i, &sym);
        if (GELF_ST_TYPE(sym.st_info) == STT_FUNC) {
            const char *name = elf_strptr(elf, shdr.sh_link, sym.st_name);
            if (name) {
                outFile << i << ":" << name << std::endl;
            }
        }
    }

    // Close the output file and cleanup
    outFile.close();
    elf_end(elf);
    close(fd);
}

/**
 * @brief Main function that parses the command line arguments and calls the function to extract function names.
 * @example ./library_func_dump <shared-library> <output-file>
 */
int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <shared-library> <output-file>" << std::endl;
        return 1;
    }

    extractFunctionNames(argv[1], argv[2]);
    return 0;
}
