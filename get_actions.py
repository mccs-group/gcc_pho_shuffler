import ctypes
import pathlib
import random

def make_c_array(lst):
    return (ctypes.c_char_p * len(lst))(*[x.encode() for x in lst])

def make_list(arr, size):
    return [elem.decode() for elem in arr[:size.value]]

def setuplib(name):
    lib = ctypes.CDLL(libname)
    lib.get_new_action_space.argtypes = [ctypes.POINTER(ctypes.c_char_p), ctypes.POINTER(ctypes.c_char_p), ctypes.c_int32, 
                                            ctypes.c_int32, ctypes.c_int32, ctypes.c_int32, ctypes.POINTER(ctypes.c_size_t)]

    lib.get_new_action_space.restype = ctypes.POINTER(ctypes.c_char_p)

    return lib

def get_action_list(old_action_list, used_actions, libactions, start_prop, end_prop):
    size = ctypes.c_size_t()
    new_action_byte_array = libactions.get_new_action_space(make_c_array(old_action_list), make_c_array(used_actions), len(old_action_list), len(used_actions),
                                                             start_prop, end_prop, ctypes.byref(size))
    return make_list(new_action_byte_array, size)

if __name__ == "__main__":

    libname = pathlib.Path().absolute() / "actions.so"
    lib = setuplib(libname)

    result = ["cprop", "pta", "alias", "loop"]
    used = []

    print ("the fun starts")
    for i in range (1000):
        result = get_action_list(result, used, lib, 552, 0)
        # print("Action space:")
        # print(result)

        index = random.randrange(0, len(result))
        # print(f"Index: {index}, len: {len(result) - 1}")

        # print("Used vec")
        used.append(result[index])
        # print(used)