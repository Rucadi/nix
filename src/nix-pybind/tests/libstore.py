import nixpy.libstore as nlu

def get_storedir_test():
    context = None  
    nix_libstore_init(context)
    store = nlu.nix_store_open(context, "", [])
    
    def callback(result, _):
        print("Store directory:", result)
    
    nlu.nix_store_get_storedir(context, store, callback)
    
    nlu.nix_store_free(store)

if __name__ == "__main__":
    get_storedir_test()
