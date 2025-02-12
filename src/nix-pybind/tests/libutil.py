
import nixpy.libutil as nix_util

def callback_function(value, user_data):
    print(f"Callback received value: {value}")
    user_data.append(value)  # Store the value for validation

# Create a Nix context
context = nix_util.nix_c_context_create()
if not context:
    print("Failed to create Nix context")
    exit(1)

# Initialize the Nix library
if nix_util.nix_libutil_init(context) != nix_util.NixErr.NIX_OK:
    print("Failed to initialize Nix library")
    nix_util.nix_c_context_free(context)
    exit(1)

# Set a test setting key-value pair
key = "setting-name"
value = "new-value"
res = nix_util.nix_setting_set(context, key, value) 
if res != nix_util.NixErr.NIX_OK:
    print("Failed to set test setting")
    print(res)
    nix_util.nix_c_context_free(context)
    exit(1)

# Prepare a list to store callback results
callback_results = []

# Get the setting using nix_setting_get and the callback function
if nix_util.nix_setting_get(context, key, callback_function, callback_results) != nix_util.NixErr.NIX_OK:
    print("Failed to get test setting")
    nix_util.nix_c_context_free(context)
    exit(1)

# Verify the callback received the expected value
if callback_results and callback_results[0] == value:
    print("Test passed: Retrieved value matches expected value.")
else:
    print("Test failed: Retrieved value does not match expected value.")

