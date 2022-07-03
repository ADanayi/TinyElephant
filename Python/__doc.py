# In the name of Allah

import typing as T


class ReadField:
    def __init__(self, parent_doc: "Doc", parent_position: int) -> None:
        self.__parent = parent_doc
        self.__position = parent_position

        self.__name = ''
        self.__len_str = ''
        self.__bytesL = list(self.__parent.bytes[self.__position:])
        self.__data_len = None
        self.__pure_len = 0
        self.__cached_val = False

        try:
            self.__process()
            self.__ok = True
        except:
            self.__pure_len = 0
            self.__ok = False

    def __process(self):
        # fetch name
        self.__pure_len = 0
        while True:
            ch = self.__bytesL[0]
            del self.__bytesL[0]
            self.__pure_len += 1
            if ch == ord('\0'):
                break
            else:
                self.__name += chr(ch)

        # fetch len
        while True:
            ch = self.__bytesL[0]
            del self.__bytesL[0]
            self.__pure_len += 1
            if ch == ord('\0'):
                break
            else:
                self.__len_str += chr(ch)
        self.__data_len = int(self.__len_str)

        # fetch data
        self.__pure_len += self.__data_len

    def __bool__(self) -> bool:
        return self.__ok

    @property
    def doc(self) -> "Doc":
        return self.__parent

    @property
    def name(self) -> str:
        return self.__name

    @property
    def data(self) -> bytes:
        if not self.__ok:
            raise Exception('Field is empty')
        return bytes(self.__bytesL)

    @property
    def pure_size(self) -> int:
        return self.__pure_len

    def __to_str(self) -> str:
        if not self.__ok:
            raise Exception('Field is empty')
        else:
            if self.__bytesL[-1] == ord('\0'):
                d = self.__bytesL[0:-1]
            else:
                d = self.__bytesL
            return bytes(d).decode()

    def __int__(self) -> int:
        if not self.__ok:
            raise Exception('Field is empty')
        else:
            return int(str(self))

    def __float__(self) -> float:
        if not self.__ok:
            raise Exception('Field is empty')
        else:
            return float(str(self))

    @property
    def val(self) -> T.Union[float, int, str, bytes, None]:
        if self.__cached_val == False:
            self.__cached_val = self.__val()
        return self.__cached_val

    def __val(self) -> T.Union[float, int, str, bytes, None]:
        """Automatically scan the data and determines the type.
Returns None if the field is not loaded successfully"""
        if not self.__ok:
            return None
        try:
            s = self.__to_str()
            try:
                i = int(s)
                return i
            except:
                try:
                    f = float(s)
                    return f
                except:
                    return s
        except:
            return self.data

    def __len__(self) -> int:
        if self.__data_len != None:
            return self.__data_len
        else:
            return 0

    def __repr__(self) -> str:
        if self.__ok:
            return f'field(name={self.name}, {self.__data_len} bytes, {type(self.val)})'
        else:
            return 'field(error)'

    def __eq__(self, __o: object) -> bool:
        if type(__o) in (None, str, float, int):
            return self.val == __o
        elif type(__o) == ReadField:
            return self.name == __o.name and self.data == __o.data
        return False


class Doc:
    def __init__(self, file_path: str) -> None:
        self.__path = file_path
        self.__D = {}
        with open(self.__path, 'rb') as file:
            self.__bytes = file.read()
        cursor = 0
        self.__names = []
        while True:
            field = ReadField(self, cursor)
            if field:
                self.__D[field.name] = field
                self.__names.append(field.name)
                cursor += field.pure_size
            else:
                break
        self.__names = tuple(self.__names)

    @property
    def names(self) -> tuple:
        return self.__names

    @property
    def bytes(self) -> bytes:
        return self.__bytes

    @property
    def path(self) -> str:
        return self.__path

    @property
    def dict(self) -> dict:
        return self.__D

    def __getitem__(self, item) -> ReadField:
        return self.__D[item]

    def __contains__(self, kw: str) -> bool:
        return kw in self.__D

    def __iter__(self):
        self.__iter_ind = 0
        return self

    def __len__(self):
        return len(self.__names)

    def __next__(self):
        if self.__iter_ind >= len(self.__names):
            raise StopIteration
        self.__iter_ind += 1
        return self.__D[self.__names[self.__iter_ind - 1]]
