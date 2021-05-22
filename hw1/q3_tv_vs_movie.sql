select  t.type, count(*) as c
from titles as t
group by t.type
order by c asc;
