import ctypes
import pathlib
import random
import time
import os

# prepares python list of strings for c function
def make_c_array(lst):
    return (ctypes.c_char_p * len(lst))(*[x.encode() for x in lst])

# makes a python list from c-like array of strings
def make_list(arr, size):
    return [elem.decode() for elem in arr[:size.value]]

# returns a lib object, which is necessary for the rest of the functions
def setuplib(name = None):
    if (name == None):
        name = os.path.dirname(__file__) + "/libactions.so"
    lib = ctypes.CDLL(str(name))
    lib.get_new_action_space.argtypes = [ctypes.POINTER(ctypes.c_char_p), ctypes.POINTER(ctypes.c_char_p), ctypes.c_int32,
                                            ctypes.c_int32, ctypes.c_int32, ctypes.POINTER(ctypes.c_size_t)]

    lib.get_new_action_space.restype = ctypes.POINTER(ctypes.c_char_p)

    lib.get_pass_list.argtypes = [ctypes.c_char_p]
    lib.get_pass_list.restype = ctypes.c_int32

    lib.valid_pass_seq.argtypes = [ctypes.POINTER(ctypes.c_char_p), ctypes.c_int32, ctypes.c_int32]
    lib.valid_pass_seq.restype = ctypes.c_int32

    lib.make_valid_pass_seq.argtypes = [ctypes.POINTER(ctypes.c_char_p), ctypes.c_int32, ctypes.c_int32, ctypes.POINTER(ctypes.c_size_t)]
    lib.make_valid_pass_seq.restype = ctypes.POINTER(ctypes.c_char_p)

    lib.get_action_space_by_property.argtypes = [ctypes.c_size_t, ctypes.c_size_t, ctypes.c_int32, ctypes.POINTER(ctypes.c_size_t)]
    lib.get_action_space_by_property.restype = ctypes.POINTER(ctypes.c_char_p)

    lib.get_list_by_list_num.argtypes = [ctypes.c_int32, ctypes.POINTER(ctypes.c_size_t)]
    lib.get_list_by_list_num.restype = ctypes.POINTER(ctypes.c_char_p)

    lib.get_property_by_history.argtypes = [ctypes.POINTER(ctypes.c_char_p), ctypes.c_int32, ctypes.c_int32, ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_size_t)]
    lib.get_property_by_history.restype = None

    lib.set_path.argtypes = [ctypes.c_char_p]
    lib.set_path.restype = None

    lib.get_shuffled_list.argtypes = [ctypes.c_int32, ctypes.POINTER(ctypes.c_size_t)]
    lib.get_shuffled_list.restype = ctypes.POINTER(ctypes.c_int32)

    lib.set_include_used.argtypes = [ctypes.c_int32]
    lib.set_include_used.restype = None

    lib.if_in_loop.argtypes = [ctypes.c_size_t]
    lib.if_in_loop.restype = ctypes.c_int32

    lib.set_path(str(name).encode())

    return lib

def set_include_used(lib, flag):
    if (flag):
        lib.set_include_used(1)
    else:
        lib.set_include_used(0)

def get_pass_list(lib, name : str):
    return int(lib.get_pass_list(name.encode()))

def get_shuffled_list(lib, list_num: int):
    size = ctypes.c_size_t()
    arr = lib.get_shuffled_list(list_num, ctypes.byref(size))
    return arr[:size.value]

def get_action_space_by_property(lib, list_num, property_state_orig, property_state_custom):
    size = ctypes.c_size_t()
    arr = lib.get_action_space_by_property(ctypes.c_size_t(property_state_orig), ctypes.c_size_t(property_state_custom), list_num, ctypes.byref(size))
    return make_list(arr, size)

def get_list_by_list_num(lib, list_num):
    size = ctypes.c_size_t()
    arr = lib.get_list_by_list_num(list_num, ctypes.byref(size))
    return make_list(arr, size)

def get_property_by_history(lib, pass_history, list_num):
    orig = ctypes.c_size_t()
    custom = ctypes.c_size_t()
    lib.get_property_by_history(make_c_array(pass_history), len(pass_history), list_num, ctypes.byref(orig), ctypes.byref(custom))
    return (orig.value, custom.value)

def in_loop(lib, custom_prop):
    return lib.if_in_loop(custom_prop)

# receives lib object, action list from previous step (could be empty), used list(could be empty), and number of list from which to take passes
# returns a list of available passes
def get_action_list(libactions, old_action_list, used_actions, list_num):
    size = ctypes.c_size_t()
    new_action_byte_array = libactions.get_new_action_space(make_c_array(old_action_list), make_c_array(used_actions), len(old_action_list), len(used_actions),
                                                             list_num, ctypes.byref(size))
    return make_list(new_action_byte_array, size)


# receives a lib object, passes sequence to check, and number of list from which these passes are
# return 0 if ok, a number of failed pass
def valid_pass_seq(lib, lst, list_num):
    return int(lib.valid_pass_seq(make_c_array(lst), len(lst), list_num))

def make_valid_pass_seq(lib, lst, list_num):
    size = ctypes.c_size_t()
    return make_list(lib.make_valid_pass_seq(make_c_array(lst), len(lst), list_num, ctypes.byref(size)), size)
