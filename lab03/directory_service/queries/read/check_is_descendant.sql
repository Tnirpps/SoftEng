WITH RECURSIVE parent_chain AS (
    SELECT uuid, parent_uuid
    FROM directories
    WHERE uuid = $1
    UNION ALL
    SELECT d.uuid, d.parent_uuid
    FROM directories d
    INNER JOIN parent_chain pc ON d.uuid = pc.parent_uuid
)
SELECT EXISTS(
    SELECT 1 FROM parent_chain WHERE uuid = $2
) as is_descendant
