WITH RECURSIVE descendants AS (
    SELECT uuid FROM directories WHERE uuid = $1
    UNION ALL
    SELECT d.uuid FROM directories d
    INNER JOIN descendants desc ON d.parent_uuid = desc.uuid
)
SELECT uuid FROM descendants
