/*
* Elevator Look
* Reference: http://classes.engr.oregonstate.edu/eecs/fall2011/cs411/proj03.pdf
* 
*/
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

struct sstf_data{
	struct list_head queue;
	int current_direction;
}

//done
static void look_merged_requests (struct request_queue *q, struct request *rq, struct request *next)
{
    list_del_init(&next->queuelist);
}

//done
static int look_dispatch(struct request_queue *q, int force)
{
	struct look_data *nd = q->elevator->elevator_data;
	if (!list_empty(&nd->queue)) {
		struct request *rq = list_entry(nd->queue.next, struct request, queuelist);
		list_del_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);
		return 1;
	}
	return 0;
}

//Needs to be done
static void look_add_request(struct request_queue *q, struct request *rq)
{
	struct look_data *nd = q->elevator->elevator_data;
	struct request *next, *previous, *current;
	
	//Check to see if look_data is empty
	printk("Attempting to add request \n");
	if(list_empty(&nd->queue))
	{
		// If empty add request to the queue
		printk("Empty! \n");
		list_add_tail(&rq->queuelist, &nd->queue);
	}
	// Otherwise now we need to find a spot
	else
	{
		//find spot to put the request in
		next = list_entry(nd->queue.next, struct request, queuelist);
		previous = list_entry(nd->queue.prev, struct request, queuelist);
		while(blk_rq_pos(rq) > blk_rq_pos(next)){
			next = list_entry(next->queuelist.next, struct request, queuelist);
			prev = list_entry(prev->queuelist.prev, struct request, queuelist);
		}
		list_add(&rq->queuelist, &prev->queuelist);
		printk("Added! \n");
	}
}

//done
static struct *look_former_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = -> q->elevator_data;
	if (rq->queuelist.prev == %nd->queue)
		reutrn NULL;
	return list)entry(rq->queuelist.prev, struct request, queuelist);
}

//done
static struct *look_latter_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = q->elevator->elevator_data;
	if(rq->queuelist.next == &nd.queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

//done
static int look_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct look_data *nd;
	struct elevator_queue *eq;
	
	eq = elevator_alloc(q, e);
	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	
	if (!eq)
		return -ENOMEM;
	if(!nd)
	{
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = nd;
	INIT_LIST_HEAD(&nd->queue);
	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);
	return 0;
}

//done
static void look_exit_queue(struct elevator_queue *e)
{
	struct look_data *nd = e->elevator_data;
    BUG_ON(!list_empty(&nd->queue));
    kfree(nd);
}

//done
static struct elevator_type elevator_look = {
    .ops = {
        .elevator_merge_req_fn 		= look_merged_requests,
        .elevator_dispatch_fn 		= look_dispatch,
        .elevator_add_req_fn 		= look_add_request,
        .elevator_former_req_fn 	= look_former_request,
        .elevator_latter_req_fn 	= look_latter_request,
        .elevator_init_fn 		= look_init_queue, 
        .elevator_exit_fn 		= look_exit_queue,
    },
    .elevator_name = "look",
    .elevator_owner = THIS_MODULE,
};

//done
static int __init look_init(void)
{
	return elv_register(&elevator_noop);
}

//done
static void __exit look_exit(void)
{
	elv_unregister(&elevator_noop);
}

module_init(noop_init);
module_exit(noop_exit);


MODULE_AUTHOR("Chase Coltman, Alec Zitzelberger");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LOOK IO scheduler");