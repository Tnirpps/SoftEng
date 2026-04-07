SELECT EXISTS(
    SELECT 1 FROM directories
    WHERE parent_uuid = $1
) as has_children
