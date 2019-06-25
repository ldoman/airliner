"""
 
    Copyright (c) 2018 Windhover Labs, L.L.C. All rights reserved.
 
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
 
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. Neither the name Windhover Labs nor the names of its contributors 
     may be used to endorse or promote products derived from this software
     without specific prior written permission.
 
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

"""

"""
ElfReader is a tool for parsing the DWARF and DIE sections of an ELF file into
a SQLite database.

Usage:
    From the command line, the expected usage of ElfReader looks like this:
        $ elf_reader --files ELF.so --database db.sqlite

    This will load the ELF.so file into the database (assuming no errors were
    generated), and the user can then run Explain or another tool to interpret
    the ELF DIE sections.

Code Limitations:
    ELF files to be parsed must conform to the following requirements:
    1. All relevant typedefs, structs, and unions must be defined in the top-
        level of a file. ElfReader will not look inside function or namespace
        definitions.

Notes:
    New developers will probably want to use "objdump XXX.so --dwarf" to view
    the DWARF information while debugging or adding new features to understand
    how the DWARF sections look in practice. The Unix International
    "dwarf-2.0.0.pdf" is also a good guide.
"""

import argparse
import hashlib
import json
import logging
import os
import sqlite3
import sys
from logging import Logger
import traceback

from elftools.elf.elffile import ELFFile

from explain.explain_error import ExplainError
from explain.loggable import Loggable


class ElfReaderError(ExplainError):
    """Base class for ElfReader Errors."""

class DuplicateElfError(ElfReaderError):
    """Error for duplicate parsed ELF """


class ElfReader(Loggable):
    """A class for parsing and storing data represented in ELF files.

    ElfReader gives the user the ability to load an ELF generated by GCC with
    the debugging option (-g) into a database, which can then be easily used by
    other tools such as Explain to interpret the data.
    """

    def __init__(self, database, logger: Logger = None) -> None:
        super().__init__(logger)
        self.database = database
        self.create_tables()

    @staticmethod
    def checksum(file_name, block_size=8192):
        """Return the checksum of the file at file_name.

        The name of the algorithm is prepended to the checksum digest. Future
        changes to this method may change the algorithm used, the prepended name
        should be updated to reflect that.
        """
        check = hashlib.md5()
        with open(file_name, 'rb') as fp:
            while True:
                chunk = fp.read(block_size)
                if not chunk:
                    break
                check.update(chunk)
        return b'md5' + check.digest()

    def create_tables(self):
        """Creates the tables required for ElfReader to correctly load ELF data.

        The rest of the ElfReader code assumes that if the tables are already
        present that they have the same schema as in this method.
        """
        c = self.database.cursor()
        c.execute(
            'CREATE TABLE IF NOT EXISTS elfs ('
            'id INTEGER PRIMARY KEY,'
            'name TEXT UNIQUE NOT NULL,'
            'checksum TEXT NOT NULL,'
            'date DATETIME NOT NULL DEFAULT(CURRENT_TIMESTAMP),'
            'little_endian BOOLEAN NOT NULL'
            ');')
        c.execute(
            'CREATE TABLE IF NOT EXISTS symbols ('
            'id INTEGER PRIMARY KEY,'
            'elf INTEGER NOT NULL,'
            'name TEXT NOT NULL,'
            'byte_size INTEGER NOT NULL,'
            'FOREIGN KEY(elf) REFERENCES elfs(id),'
            'UNIQUE (elf, name)'
            ')')
        c.execute(
            'CREATE TABLE IF NOT EXISTS fields('
            'id INTEGER PRIMARY KEY,'
            'symbol INTEGER NOT NULL,'
            'name TEXT NOT NULL,'
            'byte_offset INTEGER NOT NULL,'
            'type INTEGER,'
            'multiplicity INTEGER NOT NULL,'
            'FOREIGN KEY (symbol) REFERENCES symbols(id),'
            'FOREIGN KEY (type) REFERENCES symbols(id),'
            'UNIQUE (symbol, name)'
            ')')
        c.execute(
            'CREATE TABLE IF NOT EXISTS bit_fields('
            'field INTEGER PRIMARY KEY,'
            'bit_size INTEGER NOT NULL,'
            'bit_offset INTEGER NOT NULL,'
            'FOREIGN KEY (field) REFERENCES fields(id)'
            ') WITHOUT ROWID')
        c.execute(
            'CREATE TABLE IF NOT EXISTS enumerations('
            'symbol INTEGER NOT NULL,'
            'value INTEGER NOT NULL,'
            'name TEXT NOT NULL,'
            'FOREIGN KEY (symbol) REFERENCES symbols(id),'
            'PRIMARY KEY (symbol, value)'
            ') WITHOUT ROWID')
        c.close()

    @property
    def dump(self):
        """Return a string representing the SQL commands that could replicate
        the database."""
        return '\n'.join(line for line in self.database.iterdump())

    def insert_elf(self, file_name, module_name = None):
        """Insert an ELF file and symbols into ElfReader.

        Return True if successful.
        """
        # Checksum and load ELF
        try:
            stream = open(file_name, 'rb')
        except FileNotFoundError:
            print('No such file: {!r}'.format(file_name))
            return False
        elf = ELFFile(stream)
        checksum = self.checksum(file_name)
        name = module_name if module_name else os.path.basename(file_name)

        # Insert ELF file into elfs table.
        c = self.database.cursor()
        try:
            # Note: sqlite does not store binary data. Must use sqlite.Binary
            # to pass in checksum.
            c.execute('INSERT INTO elfs(name, checksum, little_endian) '
                      'VALUES (?, ?, ?)',
                      (name, sqlite3.Binary(checksum), elf.little_endian))
        except sqlite3.IntegrityError as e:
            # Check if the error is due to duplicate entry
            c.execute('SELECT date FROM elfs WHERE name=? AND checksum=?',
                      (name, sqlite3.Binary(checksum)))
            duplicate = c.fetchone()
            if duplicate:
                raise DuplicateElfError('{!r} matched previously loaded ELF uploaded '
                                        'on {}'.format(name, duplicate[0])) from e
            
            # Check if the error is due to duplicate name, but mismatched checksum
            c.execute('SELECT date FROM elfs WHERE name=?',(name,))
            old_version = c.fetchone()
            if old_version:
                raise ElfReaderError('Old version of {!r} was previously loaded '
                                     'on {}'.format(name, old_version[0])) from e

            # Raise original exception if neither of above cases are true
            raise e

        # Insert symbols from ELF
        elf_id = c.lastrowid
        elf_view = ElfView(self.database, elf_id, self.logger)
        elf_view.insert_symbols_from_elf(elf)
        return True


class ElfView(Loggable):
    """A class with helper methods for inserting into the database.

    Once ElfView has been constructed, the insert_symbols_from_elf method is the
    primary entry point of this class. This method goes through the ELF and adds
    every symbol it can find into the database.

    The public insert_* methods perform the actual SQL operation, while the
    private methods _tag_* deal with the parsing of individual DIE elements from
    the ELF file. Each method is deals with a specific tag. The _symbol_requires
    method is the central tie-in for adding an arbitrary DIE.
    """
    ENCODING = 'utf-8'

    def __init__(self, database, elf_id, logger=None):
        super().__init__(logger)
        self.database = database
        self.elf_id = elf_id
        # Because of cu_offset do not multi-thread this.
        self.cu_offset = None

    def insert_bit_field(self, field_id, bit_size, bit_offset):
        """Insert a bit field into the database.

        The bit_fields table is constructed without a ROWID, thus there is no
        additional information to return to the user if the insertion succeeded.

        This method does not check for a preexisting bit field, but since adding
        the same field multiple times is not supported anyway this should raise
        a SQLite error.
        """
        self.debug('insert_bit_field({!r}, {}, {})'.format(
            field_id, bit_size, bit_offset))
        self.database.execute(
            'INSERT INTO bit_fields(field, bit_size, bit_offset) '
            'VALUES (?, ?, ?)', (field_id, bit_size, bit_offset))

    def insert_enumeration(self, symbol_id, value, name):
        """Insert an enumeration value for a symbol into the database.

        The enumerations table is constructed without a ROWID, thus there is no
        additional information to return to the user if the insertion succeeded.

        This method does not check for a preexisting enumeration for the symbol
        with the same value, but since adding symbols multiple times is not
        supported anyway this should raise a SQLite error.
        """
        self.debug('insert_enumeration({!r}, {}, {})'.format(
            symbol_id, value, name))
        self.database.execute('INSERT INTO enumerations(symbol, value, name) '
                              'VALUES (?, ?, ?)', (symbol_id, value, name))

    def insert_field(self, symbol_id, name, byte_offset, kind, multiplicity=0,
                     allow_void=False):
        """Insert a field of a symbol into the database.

        Return the field row id.

        This method does not check for a preexisting field for the symbol with
        the same name, but since adding symbols multiple times is not supported
        anyway this should raise a SQLite error.
        """
        self.debug('insert_field({!r}, {}, {}, {})'.format(
            symbol_id, name, byte_offset, kind, multiplicity))
        if kind is None and not allow_void:
            raise ElfReaderError('Attempted to add a void field type without '
                                 'explicit override.')
        c = self.database.cursor()
        c.execute('INSERT INTO fields(symbol, name, byte_offset, type, '
                  'multiplicity) VALUES (?, ?, ?, ?, ?)',
                  (symbol_id, name, byte_offset, kind, multiplicity))
        field_id = c.lastrowid
        c.close()
        return field_id

    def insert_symbol(self, name, byte_size):
        """Insert a symbol with name and byte_size.

        Return the symbol row id.

        This method does not check for a preexisting symbol with the same name.
        Users should check if the symbol is already inserted by calling
        symbol().
        """
        self.debug('insert_symbol({!r}, byte_size={})'.format(
            name, byte_size))
        c = self.database.cursor()
        c.execute('INSERT INTO symbols(elf, name, byte_size)'
                  'VALUES(?, ?, ?)', (self.elf_id, name, byte_size))
        symbol_id = c.lastrowid
        c.close()
        return symbol_id

    def field(self, symbol_id, name):
        """Select a field row id by its symbol row id and name."""
        field_id = self.database.execute(
            'SELECT id FROM fields WHERE symbol=? AND name=?',
            (symbol_id, name)).fetchone()
        return field_id[0] if field_id is not None else None

    def symbol(self, name):
        """Return a symbol row id by its name."""
        symbol_id = self.database.execute(
            'SELECT id FROM symbols WHERE elf=? AND name=?',
            (self.elf_id, name)).fetchone()
        return symbol_id[0] if symbol_id is not None else None

    def insert_symbols_from_elf(self, elf):
        """Insert every symbol found in the ELF into the database.

        Iterates over each compilation unit in the ELF to rucurse through each
        DIE it can find.
        """
        # print(elf.header)
        dwarf = elf.get_dwarf_info()

        for i, cu in enumerate(dwarf.iter_CUs()):
            self.debug('CU #{}: {}'.format(i, cu.header))
            top = cu.get_top_DIE()
            dies = {c.offset - cu.cu_offset: c for c in top.iter_children()}
            # I don't like this. But it is a pain to anything else.
            self.cu_offset = cu.cu_offset

            for child in top.iter_children():
                self._symbol_requires(dies, child.offset - self.cu_offset)

    def _symbol_requires(self, dies, die_offset, typedef=None):
        """This is the central tie-in for adding a DIE to the database.

        When a DIE refers to another DIE by address, call _symbol_requires with
        the appropriate offset to route the call to create or find the existing
        symbol at that offset. User code should be wary of interpreting the DIE
        information of child symbols themselves, as the symbol may have already
        been inserted.
        """
        self.debug('_symbol_requires 0x{:x} (typedef={!r})'.format(
            die_offset, typedef))
        try:
            symbol = dies[die_offset]
        except KeyError:
            # Find possible enclosing tag
            closest = sorted(filter(lambda k: k < die_offset, dies.keys()))[-1]
            self.error(
                'Could not locate DIE at 0x{:x}. The previous tag that '
                'ElfReader recognizes is a {} at 0x{:x}.'
                .format(die_offset, dies[closest].tag, closest))
            return None
        if isinstance(symbol, int):
            self.debug('Found inserted symbol id = {}'.format(symbol))
            return symbol
        known_tags = {
            'DW_TAG_array_type': self._tag_array_type,
            'DW_TAG_base_type': self._tag_base_type,
            'DW_TAG_enumeration_type': self._tag_enumeration_type,
            'DW_TAG_pointer_type': self._tag_pointer_type,
            'DW_TAG_structure_type': self._tag_structure_type,
            'DW_TAG_typedef': self._tag_typedef,
            'DW_TAG_union_type': self._tag_union_type,
            # Known Skipped Tags
            'DW_TAG_class_type': self._tag_skip,
            'DW_TAG_const_type': self._tag_skip,
            'DW_TAG_imported_module': self._tag_skip,
            'DW_TAG_namespace': self._tag_skip,
            'DW_TAG_reference_type': self._tag_skip,
            'DW_TAG_rvalue_reference_type': self._tag_skip,
            'DW_TAG_restrict_type': self._tag_skip,
            'DW_TAG_subprogram': self._tag_skip,
            'DW_TAG_subroutine_type': self._tag_skip,
            'DW_TAG_unspecified_type': self._tag_skip,
            'DW_TAG_variable': self._tag_skip,
            'DW_TAG_volatile_type': self._tag_skip,
            'DW_TAG_dwarf_procedure': self._tag_skip
        }
        try:
            callback = known_tags[symbol.tag]
        except KeyError as e:
            raise ElfReaderError(
                'symbol_requires can\'t handle {} at DIE 0x{:x}\n{}'
                    .format(symbol.tag, die_offset, symbol)) from e
        return callback(dies, symbol.offset, typedef=typedef)

    def _symbol_byte_size(self, symbol_id):
        """Get the byte size of a symbol."""
        self.debug('_symbol_byte_size {}'.format(symbol_id))
        if isinstance(symbol_id, int):
            size = self.database.execute('SELECT byte_size FROM symbols WHERE '
                                         'id=?', (symbol_id,)).fetchone()[0]
        else:
            raise ElfReaderError('Can\'t get size of symbol that has not been '
                                 'added.')
        return size

    def _tag_array_type_multiplicity(self, die, die_offset):
        """Look at a DIE and return the multiplicity of the array represented by
        that DIE.

        Raises an ElfReaderError if the DIE is not an array.
        """
        multiplicity = None
        for child in die.iter_children():
            if child.tag == 'DW_TAG_subrange_type':
                try:
                    upper_bound = child.attributes['DW_AT_upper_bound'].value
                except KeyError:
                    self.warning('Skipping array with no multiplicity at DIE '
                         '0x{:x}'.format(die_offset))
                    return None
                if not isinstance(upper_bound, int):
                    self.warning(
                        'Array with unknown length declared at DIE 0x{:x}'
                            .format(die_offset))
                    return -1
                length = upper_bound + 1
                if multiplicity is None:
                    multiplicity = length
                else:
                    multiplicity *= length
            else:
                raise ElfReaderError('Unknown array child at:\n' + str(die))
        return multiplicity

    def _tag_array_type(self, dies, die_offset, typedef=None):
        """Insert an array into the database."""
        self.debug('_tag_array_type 0x{:x}'.format(die_offset))
        die = dies[die_offset - self.cu_offset]
        array_type = die.attributes['DW_AT_type'].value
        array_type_id = self._symbol_requires(dies, array_type)
        if array_type_id is None:
            self.warning('Skipping array of unknown type at DIE 0x{:x}'
                         .format(die_offset))
            return None
        array_type_name, unit_byte_size = self.database.execute(
            'SELECT name, byte_size FROM symbols WHERE id=?',
            (array_type_id,)).fetchone()
        multiplicity = self._tag_array_type_multiplicity(die, die_offset)
        if multiplicity is None:
            self.warning('Skipping array of unknown length at DIE 0x{:x}'
                         .format(die_offset))
            return None
        array_name = 'array_{}_{}'.format(array_type_name, multiplicity)
        symbol_size = unit_byte_size * multiplicity
        symbol_id = self.symbol(array_name)
        if symbol_id is not None:
            # Symbol exists, arrays of the same stuff are the same.
            return symbol_id
        symbol_id = self.insert_symbol(array_name, symbol_size)
        self.insert_field(symbol_id, '[array]', 0, array_type_id, multiplicity)
        dies[die_offset - self.cu_offset] = symbol_id
        return symbol_id

    def _tag_base_type(self, dies, die_offset, typedef=None):
        """Insert a base type into the database."""
        self.debug('_tag_base_type 0x{:x}'.format(die_offset))
        die = dies[die_offset - self.cu_offset]
        name = die.attributes['DW_AT_name'].value.decode(ElfView.ENCODING)
        size = die.attributes['DW_AT_byte_size'].value
        symbol_id = self.symbol(name) or self.insert_symbol(name, size)
        dies[die_offset - self.cu_offset] = symbol_id
        return symbol_id

    def _tag_enumeration_type(self, dies, die_offset, typedef=None):
        """Insert an enumeration into the database."""
        self.debug('_tag_enumeration_type 0x{:x}'.format(die_offset))
        die = dies[die_offset - self.cu_offset]
        if not typedef:
            self.debug('Skipping direct enum at 0x{:x}'.format(
                die_offset))
            return
        symbol_name = typedef
        symbol_byte_size = die.attributes['DW_AT_byte_size'].value
        symbol_id = self.symbol(symbol_name)
        if not symbol_id:
            symbol_id = self.insert_symbol(symbol_name, symbol_byte_size)
            for child in die.iter_children():
                cname = child.attributes['DW_AT_name'] \
                    .value.decode(ElfView.ENCODING)
                cvalue = child.attributes['DW_AT_const_value'].value
                self.insert_enumeration(symbol_id, cvalue, cname)
            dies[die_offset - self.cu_offset] = symbol_id
        return symbol_id

    def _tag_skip(self, dies, die_offset, typedef=None):
        """Skip over a (known) tag.

        If the tag is actually unknown this method should not be called. Unknown
        tags will raise a KeyError in symbol_requires.
        """
        tag = dies[die_offset - self.cu_offset].tag
        self.debug(
            'Skipping known tag {} at 0x{:x} (typedef={!r})'
            .format(tag, die_offset, typedef))

    def _tag_structure_type(self, dies, die_offset, typedef=None):
        """Insert a structure into the database."""
        self.debug('_tag_structure_type 0x{:x}'.format(die_offset))
        return self._tag_structure_or_union_type(
            dies, die_offset, typedef=typedef, union=False)

    def _tag_structure_or_union_type(self, dies, die_offset, union,
                                     typedef=None):
        """Insert a struct or a union into the database.

        Structs and unions have very similar DIE structures which is why these
        two are combined. In practice they are very similar constructs, except
        that unions have the bit_offset of their fields as zero.
        """
        kind = 'union' if union else 'structure'
        die = dies[die_offset - self.cu_offset]
        try:
            symbol_name = die.attributes['DW_AT_name'] \
                .value.decode(ElfView.ENCODING)
        except KeyError:
            # Unnamed structure. typedef must be set to continue.
            if not typedef:
                self.debug('Skipping unnamed {} at 0x{:x}'.format(
                    kind, die_offset))
                return None
            symbol_name = typedef
        try:
            byte_size = die.attributes['DW_AT_byte_size'].value
        except KeyError:
            # Weird structs with no size deserve to be skipped.
            self.exception('{} has no size at DIE 0x{:x}'
                           .format(kind, die_offset))
            return None
        symbol_id = self.symbol(symbol_name)
        if symbol_id is not None:
            # Already exists.
            return symbol_id
        symbol_id = self.insert_symbol(symbol_name, byte_size)
        # Have to replace DIE lookup here in case of recursive struct.
        dies[die_offset - self.cu_offset] = symbol_id
        for child in die.iter_children():
            if child.tag == 'DW_TAG_member':
                try:
                    field_name = child.attributes['DW_AT_name'] \
                        .value.decode(ElfView.ENCODING)
                except KeyError:
                    self.exception('Skipping field with no name at '
                                   'DIE 0x{:x}'.format(child.offset))
                    continue
                self.debug('{} {}.{}'
                           .format(kind, symbol_name, field_name))
                byte_offset = 0 if union else \
                    child.attributes['DW_AT_data_member_location'].value
                field_type = child.attributes['DW_AT_type'].value
                field_type_id = self._symbol_requires(
                    dies, field_type, typedef=field_name)
                if field_type_id is None:
                    self.warning(
                        'Skipping field {} with unknown type at DIE 0x{:x}'
                        .format(field_name, die_offset))
                    continue
                field_id = self.insert_field(
                    symbol_id, field_name, byte_offset, field_type_id)
                # Insert bit field entry if appropriate.
                if 'DW_AT_bit_size' in child.attributes:
                    bit_size = child.attributes['DW_AT_bit_size'].value
                    bit_offset = child.attributes['DW_AT_bit_offset'].value
                    self.insert_bit_field(field_id, bit_size, bit_offset)
            else:
                # Sometimes there are nested struct/union definitions.
                dies[child.offset] = child
        return symbol_id

    def _tag_pointer_type(self, dies, die_offset, typedef=None):
        """Insert a pointer type into the database."""
        self.debug('_tag_pointer_type 0x{:x}'.format(die_offset))
        die = dies[die_offset - self.cu_offset]
        pointer_size = die.attributes['DW_AT_byte_size'].value
        try:
            pointer_type = die.attributes['DW_AT_type'].value
        except KeyError:
            self.warning('Pointer to void type at DIE 0x{:x}'
                         .format(die_offset))
            pointer_type_id = None
        else:
            pointer_type_id = self._symbol_requires(dies, pointer_type)
            if pointer_type_id is None:
                self.warning('Pointer to unknown type at DIE 0x{:x}.'
                             .format(die_offset))
        # Try to set name to "*pointer_type", otherwise to typedef.
        c = self.database.cursor()
        pointer_name = c.execute('SELECT name FROM symbols WHERE id=?',
                                 (pointer_type_id,)).fetchone()
        c.close()
        if pointer_name is None:
            if not typedef:
                self.debug('Skipping unnamed pointer type at DIE 0x{:x}'
                           .format(die_offset))
                return None
            else:
                pointer_name = typedef
        else:
            pointer_name = '*' + pointer_name[0]

        symbol_id = self.symbol(pointer_name)
        if symbol_id is not None:
            # Symbol exists
            return symbol_id
        symbol_id = self.insert_symbol(pointer_name, pointer_size)
        self.insert_field(symbol_id, '[pointer]', 0, pointer_type_id,
                          allow_void=True)
        dies[die_offset - self.cu_offset] = symbol_id
        return symbol_id

    def _tag_typedef(self, dies, die_offset, typedef=None):
        """Insert a typedef into the database.

        In the case that the symbol being typedef'd has not been added before,
        the _symbol_requires method is called with the typedef parameter filled
        with the given name of the typedef. In case the typedef'd thing has no
        name of its own (such as a struct with no tag), the typedef'd symbol
        will use a variant of this name as its own symbol name.
        """
        self.debug('_tag_typedef 0x{:x}'.format(die_offset))
        die = dies[die_offset - self.cu_offset]
        name = die.attributes['DW_AT_name'].value.decode(ElfView.ENCODING)
        try:
            td_offset = die.attributes['DW_AT_type'].value
        except KeyError:
            self.warning('Skipping typedef with no type at DIE 0x{:x}'
                         .format(die_offset))
            return None
        try:
            td_die = dies[td_offset]
        except KeyError:
            raise ElfReaderError('Cannot find DIE at offset 0x{:x}'
                                 .format(td_offset))
        if isinstance(td_die, int):
            # typedef'd thing already inserted, add reference.
            td_id = td_die
        else:
            # typedef'd thing not inserted. Do that first.
            td_id = self._symbol_requires(dies, td_offset, typedef=name)
            if td_id is None:
                self.warning('Skipping typedef to unknown type at DIE '
                             '0x{:x}'.format(die_offset))
                return None
        # Get name of typedef base type.
        td_name = self.database.execute('SELECT name FROM symbols WHERE id=?',
                                        (td_id,)).fetchone()[0]
        # If the name is the same, the typedef should fall through to the base
        # type. If the name is different then create a new symbol that refers
        # to the base type.
        if td_name == name:
            symbol_id = td_id
        else:
            symbol_id = self.symbol(name)
            if symbol_id is not None:
                # Symbol exists
                return symbol_id
            if symbol_id is None:
                byte_size = self._symbol_byte_size(td_id)
                symbol_id = self.insert_symbol(name, byte_size)
            self.insert_field(symbol_id, 'typedef', 0, td_id)
        dies[die_offset - self.cu_offset] = symbol_id
        return symbol_id

    def _tag_union_type(self, dies, die_offset, typedef=None):
        """Insert a union into the database."""
        self.debug('_tag_union_type 0x{:x}'.format(die_offset))
        return self._tag_structure_or_union_type(
            dies, die_offset, union=True, typedef=typedef)


def main():
    parser = argparse.ArgumentParser(
        description='ElfReader can understand ELF files and create tables or '
                    'output files with symbol and field names.')
    parser.add_argument('-c', '--continue', action='store_true', dest='cont',
                        help='continue adding ELF files if one fails')
    parser.add_argument('database', default=':memory:',
                        help='use or create a database on the file system')
    parser.add_argument('files', nargs='*', default=[],
                        help='elf file(s) to load')
    parser.add_argument('-q', '--no-log', action='store_true',
                        help='disables all console logging')
    parser.add_argument('--sql', action='store_true',
                        help='stdout the SQL database')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='verbose')
    parser.add_argument('--config', help='pass a config file specifying ELFs to parse')
    args = parser.parse_args()

    # Logging
    level = logging.DEBUG if args.verbose else logging.INFO
    logger = logging.getLogger()
    logger.setLevel(60 if args.no_log else level)
    ch = logging.StreamHandler(stream=sys.stdout)
    formatter = logging.Formatter('%(levelname)s - %(message)s')
    ch.setFormatter(formatter)
    logger.addHandler(ch)

    # Open database
    database = sqlite3.connect(args.database)
    elf_reader = ElfReader(database, logger=logger)

    # Insert ELF files
    loaded = True
    
    # Check if passed config file
    if args.config:
        with open(args.config, 'r') as config_json:
            config = json.load(config_json)
            for name, elf in config["Airliner"]["modules"].items():
                try:
                    logger.info('Adding ELF {}'.format(elf["elf_file"]))
                    elf_reader.insert_elf(elf["elf_file"], name)
                except DuplicateElfError:
                    logger.info('Skipping previously parsed ELF: {}'.format(name))
                    continue
                # TODO: This can be updated later to intelligently remove old 
                # elf data and parse new one
                except ElfReaderError:
                    logger.exception('CRC mismatch for previously parsed ELF: {}. '
                                     'Please create a new database.' .format(name))
                    loaded = False
                    break
                except Exception as e:
                    if args.cont:
                        logger.exception('Problem adding ELF:')
                    else:
                        traceback.print_exc()
                        loaded = False
                        break
    else:
        for file in args.files:
            try:
                logger.info('Adding ELF {}'.format(file))
                elf_reader.insert_elf(file)
            except Exception as e:
                if args.cont:
                    logger.exception('Problem adding ELF:')
                else:
                    traceback.print_exc()
                    loaded = False
                    break
    if not loaded:
        print('Errors encountered. Database not saved.')
        exit(1)

    # Debug print database
    if args.sql:
        print(elf_reader.dump)

    database.commit()
    database.close()


if __name__ == '__main__':
    main()
