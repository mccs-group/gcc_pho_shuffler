import ctypes
import pathlib
import random
import time

def make_c_array(lst):
    return (ctypes.c_char_p * len(lst))(*[x.encode() for x in lst])

def make_list(arr, size):
    return [elem.decode() for elem in arr[:size.value]]

def setuplib(name : pathlib.Path):
    lib = ctypes.CDLL(str(libname))
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

    file = open("lists/to_shuffle2.txt")
    action_space = []
    used = []

    for line in file:
        action_space.append(line.strip())

    # print("Starting Action space:")
    # print(action_space)

    # print(action_space)
    vec_len = 100

    # start = time.time()
    for i in range (vec_len):
        action_space = get_action_list(action_space, used, lib, 76079, 2)
        # print("Action space:")
        # print(action_space)

        index = random.randrange(0, len(action_space))
        # print(f"Taken: {action_space[index]}, Index: {index}, len: {len(action_space) - 1}")

        # print("Used vec")
        used.append(action_space[index])
        # print(used)

    # end = time.time()
    # print(end - start)