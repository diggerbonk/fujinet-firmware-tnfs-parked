In this branch, I'm trying to handle menu requests without having to
make any modifications to the SIO device, because that would mean 
having to make modifications for every supported device. 

Currently failing. read_menu_entry in fujiMenu.ccp will need to 
format the two-byte encodeing into the directry filename or 
something.
