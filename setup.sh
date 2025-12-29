# igloo commands can be stored in a 'profile' for easy reuse, by default this is
#   where they are stored when using `igloo -s <profile_name>`.
# If an absolute path is given instead this directory is irrelevant.
mkdir -p "$HOME/.igloo"

[ -f "$HOME/.igloo/default" ] && rm -f "$HOME/.igloo/default"
cat > "$HOME/.igloo/default" << END
salt back
fork
sha2-256
join cat\ front\ hash(pass+salt)
sha2-256
passwordify 0 \ 
END
