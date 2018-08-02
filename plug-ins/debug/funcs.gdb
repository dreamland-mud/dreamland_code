define who
    set $zz_desc=descriptor_list
    while ($zz_desc)
	set $zz_ch=$zz_desc->character
	printf "%10s: ", $zz_ch->name._M_dataplus._M_p
	p $zz_ch
	set $zz_desc=$zz_desc->next
    end
end

define fwrite_obj0
    set $zz_obj = $arg0
    set $zz_ndx = $zz_obj->pIndexData

    printf "#O\n"
    printf "Vnum %d\n", $zz_ndx->vnum
    printf "Id   %lld\n", $zz_obj->ID
    printf "Cond  %d\n", $zz_obj->condition

    if ($zz_obj->enchanted)
	printf "Enchanted\n"
    end

    if ($zz_obj->pocket._M_dataplus._M_p[0])
	printf "Pocket %s~\n", $zz_obj->pocket._M_dataplus._M_p
    end

    printf "Nest 0\n"
    
    if ($zz_obj->material)
	printf "Material %s~\n", $zz_obj->material
    end

    if ($zz_obj->name)
	printf "Name %s~\n", $zz_obj->name
    end

    if ($zz_obj->short_descr)
	printf "ShD %s~\n", $zz_obj->short_descr
    end

    if ($zz_obj->description)
	printf "Desc %s~\n", $zz_obj->description
    end
    
    if ($zz_obj->owner)
	printf "Ownr %s~\n", $zz_obj->owner
    end
    
    if ($zz_obj->extra_flags != $zz_ndx->extra_flags)
	printf "ExtF %d\n", $zz_obj->extra_flags
    end

    if ($zz_obj->wear_flags != $zz_ndx->wear_flags)
	printf "WeaF %d\n", $zz_obj->wear_flags
    end

    if ($zz_obj->weight != $zz_ndx->weight)
	printf "Wt %d\n", $zz_obj->weight
    end


    printf "Wearloc %s\n", $zz_obj->wear_loc.name._M_dataplus._M_p	
    
    if ($zz_obj->level != $zz_ndx->level)
	printf "Lev  %d\n", $zz_obj->level
    end

    if ($zz_obj->timer)
	printf "Time %d\n", $zz_obj->timer
    end

    printf "Cost %d\n", $zz_obj->cost

    if ($zz_obj->value[0] != $zz_ndx->value[0] || $zz_obj->value[1] != $zz_ndx->value[1] || $zz_obj->value[2] != $zz_ndx->value[2] || $zz_obj->value[3] != $zz_ndx->value[3] || $zz_obj->value[4] != $zz_ndx->value[4])
	printf "Val  %d %d %d %d %d\n", $zz_obj->value[0], $zz_obj->value[1], $zz_obj->value[2], $zz_obj->value[3], $zz_obj->value[4]
    end
    
    # scroll, potioin	
    # if ($zz_obj->item_type == 2 || $zz_obj->item_type == 10)
    # TODO item_type, affects
    set $zz_aff = $zz_obj->affected
    while ($zz_aff)
	printf "    Affect: %s %d %d %d %d\n", \
		    $zz_aff->type.name._M_dataplus._M_p, \
		    $zz_aff->where, \
		    $zz_aff->level, \
		    $zz_aff->location, \
		    $zz_aff->modifier
	set $zz_aff=$zz_aff->next
    end
    # TODO extra-descr
    printf "End\n"
    # TODO behavior
    printf "\n"
end

define fwrite_obj
    set $zz_head = $arg0

    if ($zz_head)
	set $zz_last = $zz_head

	while ($zz_last->next_content)
	    set $zz_last = $zz_last->next_content
	end

	while ($zz_last)
	    set $zz_curr = $zz_head
	    set $zz_prelast = 0
	    
	    while ($zz_curr != $zz_last)
		set $zz_prelast = $zz_curr
		set $zz_curr = $zz_curr->next_content
	    end

	    set $zz_last = $zz_prelast
	    fwrite_obj0 $zz_curr
	end
    end
end

define inv 
    fwrite_obj0 $arg0->carrying
    if ($zz_obj->contains)
    end
end

